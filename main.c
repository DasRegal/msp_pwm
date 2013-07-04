//******************************************************************************
//  MSP430F20x2/3 Demo - SPI full-Duplex 3-wire Slave
//
//  Description: SPI Master communicates full-duplex with SPI Slave using
//  3-wire mode. The level on P1.4 is TX'ed and RX'ed to P1.0.
//  Master will pulse slave reset for synch start.
//  ACLK = n/a, MCLK = SMCLK = Default DCO
//
//                Slave                      Master
//               MSP430F20x2/3              MSP430F20x2/3
//             -----------------          -----------------
//            |              XIN|-    /|\|              XIN|-
//            |                 |      | |                 |
//            |             XOUT|-     --|RST          XOUT|-
//            |                 | /|\    |                 |
//            |          RST/NMI|--+<----|P1.2             |
//      LED <-|P1.0             |        |             P1.4|<-
//          ->|P1.4             |        |             P1.0|-> LED
//            |         SDI/P1.7|<-------|P1.6/SDO         |
//            |         SDO/P1.6|------->|P1.7/SDI         |
//            |        SCLK/P1.5|<-------|P1.5/SCLK        |
//
//  M. Buccini / L. Westlund
//  Texas Instruments Inc.
//  October 2005
//  Built with CCE Version: 3.2.0 and IAR Embedded Workbench Version: 3.40A
//******************************************************************************

#include <msp430f2002.h>

int dx1;
int dx2;
int dx3;
int dx4;
int dx5;
int dx;
int flag;
int fl;
int usiflag;

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
    // USISRL = 0xFD;
    // USICNT = 8;
    // while (!(USIIFG & USICTL1));
    // if (USISRL == 0xFF)
    // {
    //   USISRL = 0xFD;
    //   USICNT = 8;
    //   while (!(USIIFG & USICTL1));
    //   if (USISRL == 0xFF)
    //   {
    //     USISRL = 0xFD;
    //     USICNT = 8;
    //     while (!(USIIFG & USICTL1));
    //     if (USISRL == 0xBD)
    //     {
    //       P1OUT ^= 0x01;
    //       if (fl == 0)
    //       {
    //         dx1 = 2000;
    //         fl = 1;
    //       }
    //       else
    //       {
    //         dx1 = 1000;
    //         fl = 0;
    //       }
    //     }

    //   }       
    // }

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
        dx1 = i << 8;
        usiflag = 3;
        break;
    }
    case 3:
    {
        dx1 += i;
        usiflag = 4;
        break;
    }
    case 4:
    {
        dx2 = i << 8;
        usiflag = 5;
        break;
    }
    case 5:
    {
        dx2 += i;
        usiflag = 6;
        break;
    }
    case 6:
    {
        dx3 = i << 8;
        usiflag = 7;
        break;
    }
    case 7:
    {
        dx3 += i;
        usiflag = 8;
        break;
    }
    case 8:
    {
        dx4 = i << 8;
        usiflag = 9;
        break;
    }
    case 9:
    {
        dx4 += i;
        usiflag = 10;
        break;
    }
    case 10:
    {
        dx5 = i << 8;
        usiflag = 11;
        break;
    }
    case 11:
    {
        dx5 += i;
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
      }
      break;
    }   
    case 2:
    {
      if (!flag)
      {
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
      }
      break;
    }
    case 3:
    {
      if (!flag)
      {
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
      }
      break;
    }
    case 4:
    {
      if (!flag)
      {
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
      }
      break;
    }
    case 5:
    {
      if (!flag)
      {
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
      }
      break;
    }
  }
} // CCR0_ISR