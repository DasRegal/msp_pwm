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
  #define   INT_PORT_ON   (P2IE |= CS)
  #define   INT_PORT_OFF  (P2IE &= ~CS)
  #define   INT_USI_ON    (USICTL1 |=  USIIE)
  #define   INT_USI_OFF   (USICTL1 &= ~USIIE)

#define   CS      BIT7
#define   LED     BIT6
#define   MISO    BIT6
#define   MOSI    BIT7
#define   CLK     BIT5

struct SChan
{
  int isPulse;
  int data;
  int dx;
  int isHiByte;
};

volatile struct SChan chan[N_CHANNELS];

volatile int indexChan;
volatile int indexByte;
volatile int isLock;
volatile int dx_temp;
volatile int chanel;
volatile int isChan;

void InitPWM(int n)
{
  int i;
  P1DIR = 0;
  for (i = 0; i < n; i++)
  {
    chan[i].isPulse = 1;
    chan[i].dx = 1000;
    chan[i].isHiByte = 1;
    P1DIR |= 1 << i;
    P1OUT &= ~(1 << i);
  }
  indexChan = 0;
  indexByte = 0;
  isLock = 0;
  isChan = 1;
}

void main(void)
{
  // WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer

  // Watchdog автоматически
  // перезапустит систему через 32ms.
  WDTCTL = WDT_MRST_32;

  // Устанавливаем частоту DCO на калиброванные 1 MHz.
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  USICTL0 |= USIPE7 + USIPE5 + USIOE; // Port, SPI slave
  // USICTL1 |= USIIE;                     // Counter interrupt, flag remains set
  INT_USI_OFF;
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
  // P1DIR |= BIT6;
  // P1OUT &= ~BIT6;

  P2DIR &= ~CS;
  P2SEL &= ~CS;
  P2OUT |= CS;
  P2REN |= CS;
  P2IES &= ~CS;
  P2IFG &= ~CS;

  P2SEL &= ~LED;
  P2DIR |= LED;
  P2OUT |= LED;
  P2OUT ^= LED;

  P1DIR  &= ~(MOSI + CLK);
  P1REN |= MOSI + CLK;


  P2OUT |= LED;
  INT_PORT_ON;

  P1DIR &= ~MISO;

  _BIS_SR(GIE);             // Enter LPM0 w/ interrupt
  while(1);
}

#pragma vector=USI_VECTOR
__interrupt void universal_serial_interface(void)
{
  int data;
  data = USISRL & 0xFF;
  if (isChan)
  {
    chanel = data;
    isChan = 0;
  }
  else
  {
    if (chan[chanel].isHiByte)
    {
      chan[chanel].isHiByte = 0;
      dx_temp = data << 8;
    }
    else
    {
      chan[chanel].dx = dx_temp + data;
      chan[chanel].isHiByte = 1;
      isChan = 1;   
    }
  }
 
  USISRL = 0x00;
  USICNT = 8;
  P2OUT ^= LED;
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
    
    // Сброс таймера watchdog в ноль.
    WDTCTL = WDTPW + WDTCNTCL;
  }  
} // CCR0_ISR

#pragma vector=PORT2_VECTOR
__interrupt void Port_1(void)
{
  INT_PORT_OFF;

  // Переход CS 1 -> 0
  if (P2IES & CS)
  {
    // линию DO - выход
    // P1DIR |= MISO;
    INT_USI_ON;
  }
  else
  { 
    // линию DO - вход
    // дабы исключить КЗ
    // P1DIR &= ~MISO;
    P2OUT |= LED;
    USICTL1 &= ~USIIFG;
    isChan = 1;
    INT_USI_OFF;
  }

  P2IFG &= ~CS;
  P2IES ^= CS;
  INT_PORT_ON;
}