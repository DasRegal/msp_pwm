// 
// 4-х канальный ШИМ основанный на прерывании по таймеру
// 

#include <msp430f2002.h>

/*  Объявление функций  */

int dx1;
int dx2;
int dx3;
int dx4;
int dx;
int flag;

int main(void)
{
	// отключаем сторожевой таймер
	WDTCTL = WDTPW + WDTHOLD;
	
	P1OUT = 0;
	P1DIR = BIT1 | BIT2 | BIT3 | BIT4;
	P1OUT &= ~(BIT1 | BIT2 | BIT3 | BIT4);
	
	// Устанавливаем частоту DCO на калиброванные 1 MHz.
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	
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
	_BIS_SR(GIE);

	dx1 = 1000;
	dx2 = 1600;
	dx3 = 900;
	dx4 = 2500;
	
	while(1)
	{

	}
	return 0;
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
				P1OUT |= BIT1;
				TACCR0 = dx1;
			}
			else
			{
				flag = 0;
				P1OUT &= ~BIT1;
				dx = 2;
				TACCR0 = 5000 - dx1;
			}
			break;
		}		
		case 2:
		{
			if (!flag)
			{
				flag = 1;
				P1OUT |= BIT2;
				TACCR0 = dx2;
			}
			else
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
			if (!flag)
			{
				flag = 1;
				P1OUT |= BIT3;
				TACCR0 = dx3;
			}
			else
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
			if (!flag)
			{
				flag = 1;
				P1OUT |= BIT4;
				TACCR0 = dx4;
			}
			else
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