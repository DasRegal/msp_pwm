//******************************************************************************
//
//                Slave
//               MSP430F20x2
//             -----------------          -----------------
//            |              XIN|-       |                 |
//            |                 |        |                 |
//           -|P1.0         XOUT|-       |                 |
//           -|P1.1             |        |                 |
//       PWM -|P1.2             |        |                 |
//           -|P1.3             |        |                 |
//           -|P1.4             |        |                 |
//            |         SDI/P1.7|<-------|MOSI             |
//            |                 |        |                 |
//            |        SCLK/P1.5|<-------|SCLK             |
//
// |  START  |1 Channel|2 Channel|3 Channel|4 Channel|5 Channel|
// |0xFF|0xFF|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|0xXX|
//           | HB   LB |
// result = (HB << 8) + LB
//******************************************************************************

#include <msp430f2002.h>

  #define   N_CHANNELS    5
  #define   T_CHANNEL     (20000 / N_CHANNELS)
  #define   LOCK_CHAN     (isLock = 1)
  #define   UNLOCK_CHAN   (isLock = 0)

struct SChan
{
  int isPulse;
  int data;
  int dx;
};

struct SChan chan[N_CHANNELS];

int indexChan;
int indexByte;
int isLock;
int dx_temp;

void InitPWM(int n)
{
  int i;
  P1DIR = 0;
  for (i = 0; i < n; i++)
  {
    chan[i].isPulse = 1;
    chan[i].dx = 1000;
    P1DIR |= 1 << i;
    P1OUT &= ~(1 << i);
  }
  indexChan = 0;
  indexByte = 0;
  isLock = 0;
}

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer

  // Устанавливаем частоту DCO на калиброванные 1 MHz.
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  USICTL0 |= USIPE7 + USIPE5 + USIOE; // Port, SPI slave
  USICTL1 |= USIIE;                     // Counter interrupt, flag remains set
  USICTL0 &= ~USISWRST;                 // USI released for operation
  USISRL = 0xFE;
  USICNT = 8;

  TACCR0 = 1000;
  // Разрешаем прерывание таймера по достижению значения CCR0.
  TACCTL0 = CCIE;
  TACTL = TASSEL_2 + MC_1 + TACLR; // Настройка режима работы таймера Timer_A:
                   // TASSEL_2 - источник тактов SMCLK (SubMainCLocK),
                   // по умолчанию настроенных на работу от DCO
                   // MC_1 - режим прямого счёта (до TACCR0)
                   // TACLR - начальное обнуление таймера 

  InitPWM(N_CHANNELS);
  P1DIR |= BIT6;
  P1OUT &= ~BIT6;

  _BIS_SR(LPM0_bits + GIE);             // Enter LPM0 w/ interrupt
  while(1);
}

#pragma vector=USI_VECTOR
__interrupt void universal_serial_interface(void)
{
  int data;
  data = USISRL & 0xFF;

  if (indexByte <= 1 && data == 0xFF) 
  {
    indexByte++;
    if (indexByte == 1 && data != 0xFF) indexByte = 1;
  }
  else
  if ((indexByte < (N_CHANNELS * 2 + 2)) && indexByte > 1)
  {
    if (!(indexByte % 2))
    {
      dx_temp = data << 8;
    }
    else
    {
      LOCK_CHAN;
      chan[(indexByte - 3) / 2].dx = dx_temp + data;
      UNLOCK_CHAN;
    }

    if (indexByte == (N_CHANNELS * 2 + 1)) indexByte = 0;
    else indexByte++;
  }
  else indexByte = 0;

  USISRL = 0xFE;
  USICNT = 8;
}

// 
// Обработчики прерываний
// 
#pragma vector = TIMERA0_VECTOR  
__interrupt void CCR0_ISR(void)
{
  if (!isLock) chan[indexChan].data = chan[indexChan].dx;
  if (chan[indexChan].isPulse)
  {
    chan[indexChan].isPulse = 0;
    P1OUT |= 0x01 << (indexChan);
    TACCR0 = chan[indexChan].data;
  }
  else
  {
    chan[indexChan].isPulse = 1;
    P1OUT &= ~(0x01 << (indexChan));

    if (indexChan == N_CHANNELS - 1) indexChan = 0; 
    else indexChan++;
    TACCR0 = T_CHANNEL - chan[indexChan].data;
  }  
} // CCR0_ISR