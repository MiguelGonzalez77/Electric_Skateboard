#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "GPIO_Driver.h"
#include "Servo_Driver.h"

void servo_init(uint8_t timer, uint8_t channel)
{
	GPIO_TYPE PWM_Output_Servo;
	TIM_TypeDef *timerPort = TIM2;
	uint8_t channelOffset = 0;
	
	PWM_Output_Servo.mode = OUTPUT;
	PWM_Output_Servo.mode_type = OUTPUT_ALT_FUNCTION;
	PWM_Output_Servo.speed = SPEED_50MHZ;
	
	switch(timer)
	{
		case 2:
			timerPort = TIM2;
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
			
			if(channel == 1)
			{
					PWM_Output_Servo.port = PORTA;
					PWM_Output_Servo.pin = 15;
			}
			else if(channel == 2)
			{
					PWM_Output_Servo.port = PORTA;
					PWM_Output_Servo.pin = 1;
			}
			else if(channel == 3)
			{
				PWM_Output_Servo.port = PORTA;
				PWM_Output_Servo.pin = 2;
			}
			else
			{
				PWM_Output_Servo.port = PORTA;
				PWM_Output_Servo.pin = 3;
			}
			
		break;
		
		case 3:
			timerPort = TIM3;
			RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
			
			if(channel == 1)
			{
					PWM_Output_Servo.port = PORTA;
					PWM_Output_Servo.pin = 6;
			}
			else if(channel == 2)
			{
					PWM_Output_Servo.port = PORTA;
					PWM_Output_Servo.pin = 7;
			}
			else if(channel == 3)
			{
				PWM_Output_Servo.port = PORTB;
				PWM_Output_Servo.pin = 0;
			}
			else
			{
				PWM_Output_Servo.port = PORTB;
				PWM_Output_Servo.pin = 1;
			}
			
		break;

		case 4:
			timerPort = TIM4;
			RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
			
			if(channel == 1)
			{
					PWM_Output_Servo.port = PORTB;
					PWM_Output_Servo.pin = 6;
			}
			else if(channel == 2)
			{
					PWM_Output_Servo.port = PORTB;
					PWM_Output_Servo.pin = 7;
			}
			else if(channel == 3)
			{
				PWM_Output_Servo.port = PORTB;
				PWM_Output_Servo.pin = 8;
			}
			else
			{
				PWM_Output_Servo.port = PORTB;
				PWM_Output_Servo.pin = 9;
			}
		break;
	}
	
	switch(channel)
	{
		case 1:
			channelOffset = 0;
			//enable PWM mode 1 and the preloader for channel 4
			timerPort->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE;
		break;
		
		case 2:
			channelOffset = 4;
			//enable PWM mode 1 and the preloader for channel 4
			timerPort->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE;
		break;
	
		case 3:
			channelOffset = 8;
			//enable PWM mode 1 and the preloader for channel 4
			timerPort->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE;
		break;

		case 4:
			channelOffset = 12;
			//enable PWM mode 1 and the preloader for channel 4
			timerPort->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE;
		break;	
	}
		
	init_gpio(PWM_Output_Servo);
	
	//init AFIO clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	//enable capture/compare for specified channel 
	timerPort->CCER |= (1<<channelOffset);
	
	//init the ARR 
	timerPort->CR1 |= TIM_CR1_ARPE;

	
	//enable duty cycle
	timerPort->PSC = 1440;
	timerPort->ARR = 1000; // 50Hz for servo control
	
	//enable the update generation and turn timer on
	timerPort->EGR |= TIM_EGR_UG;
	timerPort->CR1 |= TIM_CR1_CEN;
}

void servo_writeMicroseconds(int microseconds, uint8_t timer, uint8_t channel)
{
	uint16_t dutyCycle = 0;
	TIM_TypeDef *timerPort = TIM3;
	
	switch(timer)
	{
		case 2:
			timerPort = TIM2;
		break;
		case 3:
			timerPort = TIM3;
		break;
		case 4:
			timerPort = TIM4;
		break;
	}
	
	//servos have a range of 180 degrees, and the duty cycle in ms from 0-180 degrees equals
	//1ms to 2ms. Because of this, I wanted the input to be of longer range, so I have the user
	//input the time they want in microseconds from 1000-2000. The equation below sets
	//the dutyCycle variable to a proportional value from microseconds to a value that can be
	//given to the timers CCRx register. for example, if the user wants to move the servo to 0 
	//degrees, the user passes in 1000, this will be divided by 20000.0 to equal 0.05. 0.05 will 
	//be multiplied by timerPort->ARR (which is 1000) to equal 50. 50 is now a number that can
	//be set to the timers CCRx register to give a duty cycle of 5% (1ms pulse over a period of 20ms(50hz))
	dutyCycle = (uint16_t)((microseconds/20000.0)*(timerPort->ARR)); 
	
	switch(channel)
	{
		case 1:
			timerPort->CCR1 = dutyCycle;
			break;
		case 2:
			timerPort->CCR2 = dutyCycle;
			break;
		case 3:
			timerPort->CCR3 = dutyCycle;
			break;
		case 4:
			timerPort->CCR4 = dutyCycle;
			break;
	}		
}

