/* interrupted-2_G2211: Эта программа демонстрирует использование прерываний 
 * на примере переключения реле (или светодиода, в демонстрации) 
 * за период в одну минуту. Обычная программа-мигалка «Привет мир!».
 */

#include <msp430f2002.h>

/*  Объявление функций  */

int dx1;
int dx2;
int dx3;
int dx4;
int dx;
int flag;
int lock;

int hz;

int dxx1;

void del(int val)
{
  int i, j;
  for (i = 0; i < val; i++)
  {
    for (j = 0; j < 100000; i++) ;
  }
}

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;    // отключаем сторожевой таймер
    
    P1OUT = 0;                  
    P1DIR = BIT1 | BIT2 | BIT3 | BIT4 | BIT6;  // P1.6 выход на реле (или светодиод)
    P1OUT &= ~(BIT1 | BIT2 | BIT3 | BIT4 | BIT6);
    
    BCSCTL1 = CALBC1_8MHZ; // Устанавливаем частоту DCO на калиброванные 1 MHz.
    DCOCTL = CALDCO_8MHZ;
    
    TACCR0 = 1000;    // Период в 20,000 цикла, от 0 до 20,000.
    TACCTL0 = CCIE;        // Разрешаем прерывание таймера по достижению значения CCR0.
    TACTL = TASSEL_2 + ID_3 + MC_1 + TACLR; // Настройка режима работы таймера Timer_A:
                                    // TASSEL_2 - источник тактов SMCLK (SubMainCLocK),
                                    // по умолчанию настроенных на работу от DCO
                                    // MC_1 - режим прямого счёта (до TACCR0)
                                    // TACLR - начальное обнуление таймера 
    dx = 1; 
    flag = 0;
    lock = 0;
    _BIS_SR(GIE);



    dx1 = 1000;
    dx2 = 1500;
    dx3 = 2000;
    dx4 = 2000;
 
    hz = 1000;

    for(;;) { 

    }         // экономных режимов работы микроконтроллера!
} // main

/*  Обработчики прерываний  */
#pragma vector = TIMERA0_VECTOR  
__interrupt void CCR0_ISR(void) {
  switch(dx)
  {
    hz = 2000;
    case 1:
      {

        if (flag == 0)
        {
          dxx1 = 2000;
          flag = 1;
          P1OUT |= BIT1;
          TACCR0 = dxx1;
          
        }
        else
          if (flag == 1)
        {
          flag = 0;
          P1OUT &= ~BIT1;
          dx = 2;
          
          TACCR0 = 5000 - dxx1;
       
        }
        break;
      }
      
    case 2:
      {
        if (flag == 0)
        {
          flag = 1;
          P1OUT |= BIT2;
          TACCR0 = dx2;
        }
        else
          if (flag == 1)
        {
          flag = 0;
          P1OUT &= ~BIT2;
          dx = 3;
          TACCR0 = 5000 - dx2;
        }
        break;
      }
    case 3:
      {
        if (flag == 0)
        {
          flag = 1;
          P1OUT |= BIT3;
          TACCR0 = dx3;
        }
        else
          if (flag == 1)
        {
          flag = 0;
          P1OUT &= ~BIT3;
          dx = 4;
          TACCR0 = 5000 - dx3;
        }
        break;
      }
    case 4:
      {
        if (flag == 0)
        {
          flag = 1;
          P1OUT |= BIT4;
          TACCR0 = dx4;
        }
        else
          if (flag == 1)
        {
          flag = 0;
          P1OUT &= ~BIT4;
          dx = 1;
          TACCR0 = 5000 - dx4;
        }
        break;
      }
  }
        
} // CCR0_ISR