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

int dx1;
int dx2;
int dx3;
int dx4;
int dx5;
int dx_t[5];
int dx;
int flag;
int fl;
int usiflag;
int lock[5];


void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer

  // Устанавливаем частоту DCO на калиброванные 1 MHz.
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  // P1OUT =  0x10;                        // P1.4 set, else reset
  // P1REN |= 0x10;                        // P1.4 pullup
  // P1DIR = 0x01;                         // P1.0 output, else input
  //USICTL0 |= USIPE7 + USIPE6 + USIPE5 + USIOE; // Port, SPI slave
  USICTL0 |= USIPE7 + USIPE5 + USIOE; // Port, SPI slave
  USICTL1 |= USIIE;                     // Counter interrupt, flag remains set
  USICTL0 &= ~USISWRST;                 // USI released for operation
  USISRL = 0xFE;
  USICNT = 8;

  P1DIR = BIT0 | BIT1 | BIT2 | BIT3 | BIT4;
  P1OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4);

  TACCR0 = 1000;
  // Разрешаем прерывание таймера по достижению значения CCR0.
  TACCTL0 = CCIE;
  TACTL = TASSEL_2 + MC_1 + TACLR; // Настройка режима работы таймера Timer_A:
                   // TASSEL_2 - источник тактов SMCLK (SubMainCLocK),
                   // по умолчанию настроенных на работу от DCO
                   // MC_1 - режим прямого счёта (до TACCR0)
                   // TACLR - начальное обнуление таймера 

  dx = 1; 
  flag = 0;
  usiflag = 0;
  lock[0] = 0;
  lock[1] = 0;
  lock[2] = 0;
  lock[3] = 0;
  lock[4] = 0;
  _BIS_SR(GIE);

  dx1 = 1000;
  dx2 = 2000;
  dx3 = 1000;
  dx4 = 2000;
  dx5 = 1000;

  // P1OUT &= ~0x01;

  // _BIS_SR(LPM0_bits + GIE);             // Enter LPM0 w/ interrupt

  
  fl = 0;
  while(1)
  {
  }

}

#pragma vector=USI_VECTOR

__interrupt void universal_serial_interface(void)

{

int i;
  i = USISRL & 0xFF;

    switch(usiflag)
    {
      case 0:
      {
          if (i == 0xFF)
          {
            usiflag = 1;
          }
          USISRL = 0xFE;
          USICNT = 8;
          break;
      }
      case 1:
      {
          if (i == 0xFF)
          {
            usiflag = 2;
            break;
          }
          else
            usiflag = 0;
          break;
      }
      case 2:
      {   
        //dx1 = i << 8;
        dx_t[0] = i << 8;
        usiflag = 3;
        break;
      }
      case 3:
      {   
        if (!lock[0])
          // dx1 += i;
          dx1 = dx_t[0] + i;
        usiflag = 4;
        break;
      }
      case 4:
      {
        // dx2 = i << 8;
        dx_t[1] = i << 8;
        usiflag = 5;
        break;
      }
      case 5:
      {
        if (!lock[1])
          // dx2 += i;
          dx2 = dx_t[1] + i;
        usiflag = 6;
        break;
      }
      case 6:
      {
        // dx3 = i << 8;
        dx_t[2] = i << 8;
        usiflag = 7;
        break;
      }
      case 7:
      {
        if (!lock[2])
          dx3 = dx_t[2] + i;
        usiflag = 8;
        break;
      }
      case 8:
      {
        // dx4 = i << 8;
        dx_t[3] = i << 8;
        usiflag = 9;
        break;
      }
      case 9:
      {
        if (!lock[3])
          // dx4 += i;
          dx4 = dx_t[3] + i;
        usiflag = 10;
        break;
      }
      case 10:
      {
        // dx5 = i << 8;
        dx_t[4] = i << 8;
        usiflag = 11;
        break;
      }
      case 11:
      {
        if (!lock[4])
          // dx5 += i;
          dx5 = dx_t[4] + i;
        usiflag = 0;
        break;
      }

    }
    USISRL = 0xFE;
    USICNT = 8;
  

}


// 
// Обработчики прерываний
// 
#pragma vector = TIMERA0_VECTOR  
__interrupt void CCR0_ISR(void)
{
  switch(dx)
  {
    case 1:
    {
      if (!flag)
      {
        lock[0] = 1;
        flag = 1;
        P1OUT |= BIT0;
        TACCR0 = dx1;
      }
      else
      {
        flag = 0;
        P1OUT &= ~BIT0;
        dx = 2;
        TACCR0 = 4000 - dx1;
        lock[0] = 0;
      }
      break;
    }   
    case 2:
    {
      if (!flag)
      {
        lock[1] = 1;
        flag = 1;
        P1OUT |= BIT1;
        TACCR0 = dx2;
      }
      else
      {
        flag = 0;
        P1OUT &= ~BIT1;
        dx = 3;
        TACCR0 = 4000 - dx2;
        lock[1] = 0;
      }
      break;
    }
    case 3:
    {
      if (!flag)
      {
        lock[2] = 1;
        flag = 1;
        P1OUT |= BIT2;
        TACCR0 = dx3;
      }
      else
      {
        flag = 0;
        P1OUT &= ~BIT2;
        dx = 4;
        TACCR0 = 4000 - dx3;
        lock[2] = 0;
      }
      break;
    }
    case 4:
    {
      if (!flag)
      {
        lock[3] = 1;
        flag = 1;
        P1OUT |= BIT3;
        TACCR0 = dx4;
      }
      else
      {
        flag = 0;
        P1OUT &= ~BIT3;
        dx = 5;
        TACCR0 = 4000 - dx4;
        lock[3] = 0;
      }
      break;
    }
    case 5:
    {
      if (!flag)
      {
        lock[4] = 1;
        flag = 1;
        P1OUT |= BIT4;
        TACCR0 = dx5;
      }
      else
      {
        flag = 0;
        P1OUT &= ~BIT4;
        dx = 1;
        TACCR0 = 4000 - dx5;
        lock[4] = 0;
      }
      break;
    }
  }
} // CCR0_ISR