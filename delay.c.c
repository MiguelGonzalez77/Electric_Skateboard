#include "stm32f10x.h"
#include "delay.h"

volatile static int ticks_delay = 0;
volatile static int ticks_millis = 0;
volatile static uint8_t millis_flag = 0;
volatile static uint8_t delay_flag = 0;

int start_millis(void)
{
	millis_flag = 1;
	TIM2->CR1 |= TIM_CR1_CEN; //turn timer on
	
	return ticks_millis;
}

void reset_millis(void)
{
	millis_flag = 0;
	TIM2->CR1 &= ~(TIM_CR1_CEN);
	ticks_millis = 0;
}

int play_millis(int time)
{
	millis_flag = 1;
	TIM2->CR1 |= TIM_CR1_CEN; //turn timer on
	
	if(ticks_millis > time)
	{
		reset_millis();
	}
	
	return ticks_millis;
}


void init_delay(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	TIM2->PSC = 0;
	TIM2->ARR = 72; // 1Mhz for easy timing with uS and mS
	
	TIM2->CR1 |= TIM_CR1_URS;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->EGR |= TIM_EGR_UG;
	
	NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler()
{
	TIM2->SR &= ~TIM_SR_UIF;
	if(delay_flag)
	{
		ticks_delay++;
	}
	if(millis_flag)
	{
		ticks_millis++;
	}
}


void delayUs(int uS)
{
	ticks_delay = 0; 
	delay_flag = 1;
	TIM2->CR1 |= TIM_CR1_CEN; //turn timer on

	while(ticks_delay < uS){}
	
	TIM2->CR1 &= ~TIM_CR1_CEN; //turn timer off
	delay_flag = 0;
}

void delayMs(int mS)
{
	ticks_delay = 0; 
	delay_flag = 1;
	TIM2->CR1 |= TIM_CR1_CEN; //turn timer on

	while(ticks_delay < (mS * 1000)){}

	TIM2->CR1 &= ~TIM_CR1_CEN; //turn timer off
	delay_flag = 0;
}
