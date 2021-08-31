//===================================================================================================================
//																	  	CODE FOR THE SKATEBOARD
//===================================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "GPIO_Driver.h"
#include "NRF_Driver.h"
#include "delay.h"
#include "Servo_Driver.h"

#define NUM_LEDS 39 //you must add 1 to how many existing LED's you have. 
									  //For example, if you have 5 LEDs, put 6 for this number.
									  //I implemented this because sometimes, a bit or two would bleed
									  //into the next LED, causing it to light up green when the user did not want 
									  //to. This causes that last LED to deliberatly be set to zero.

#define RX_PIPE 1
#define ADDR_HIGH 0xAABBCCDD
#define ADDR_LOW 0xEE
#define EN_D_PL_W 1
#define EN_AUTO_ACK 1
#define PL_W 3
#define CHANNEL 0x7B

void init_PWM_TIM4(void); //neopixels
void TIM4_IRQHandler(void);
void strip_fill(uint32_t R, uint32_t G, uint32_t B, int start, int END);
void strip_show(void);
void strip_run(void);
void strip_rainbow(void);
void strip_fade(void);
void strip_clear(void);
void strip_ASU(void);

uint16_t map(uint16_t var, uint16_t var_low, uint16_t var_high, uint16_t low_thresh, uint16_t high_thresh);

volatile static uint16_t pwm_duty_cicle_neopixels = 0;
volatile static int currentLED = 0;
volatile static int currentRGB = 0;
volatile static int resetFlag = 0;


int static neoPixelBuffer[NUM_LEDS][24]; //this global 2D array appears in the TIM4_IRQHandler and
																				 //the strip_fill function. 

int main() 
{
	//RX MODE 
	uint8_t dataBuffer[4] = {240, 0, 0, 0};
	uint16_t servo_microseconds = 1100;
	uint16_t nrf_disconnection_counter = 0; //if this variable overflows, then 
																			 //the program knows that the user
																			 //has lost connection with the board 
	                                     //and will stop the board automatically
	
	//==========================================
	GPIO_TYPE PWM_Output_Neopixels;
	PWM_Output_Neopixels.port = PORTB;
	PWM_Output_Neopixels.pin = 9;
	PWM_Output_Neopixels.mode = OUTPUT;
	PWM_Output_Neopixels.mode_type = OUTPUT_ALT_FUNCTION;
	PWM_Output_Neopixels.speed = SPEED_50MHZ;
	//==========================================
	
	init_gpio(PWM_Output_Neopixels);
	servo_init(3, 4); //this inits a pwm signal (50hz) on timer 3 channel 4
	
	init_delay();
	init_PWM_TIM4();
	
	nrf_init_pins(PORTA, 0, PORTA, 1); //these are the CE and CSN pins on the NRF module
	nrf_init_reciever(RX_PIPE, ADDR_HIGH, ADDR_LOW, CHANNEL, EN_AUTO_ACK, EN_D_PL_W, 32);
	nrf_cmd_act_as_RX();
	nrf_cmd_listen();

	while(1)
	{ 
		//RX MODE
		
		
		if(nrf_data_available()) 
		{
			nrf_disconnection_counter = 0; //getting into this if statement means that
																	//the nrf module has a connection and 
																	//the nrf_disconnection_counter is reset
			nrf_read(dataBuffer);
			servo_microseconds = map(dataBuffer[1], 0, 255, 1100, 2000);
		}
		else
		{
			nrf_disconnection_counter += 1; //increment the nrf_disconnection_counter by 1. If the nrf module
																	 //has had no connection for around 1 second, then the
																	 //servo_microseconds variable is set to a value that
																	 //will slowly stop the skateboard. 
			if(nrf_disconnection_counter > 100) 
			{
				servo_microseconds = 1100;
				nrf_disconnection_counter = 0;
		
			}
		}
		delayMs(10);
		
		if(dataBuffer[3] != 1)
		{
			servo_writeMicroseconds(servo_microseconds, 3, 4);
		}
		else
		{
			servo_writeMicroseconds(1000, 3, 4);
		}
			
			
		if(play_millis(100*1000) == 0) //this play_millis function equals 0 after the user identified time (in us) 
																	 //has past. It will stay zero until the play_millis function is called again
		{ 
			switch(dataBuffer[2])
			{
				case 0:
					strip_fill(150, 0, 0, 0, NUM_LEDS); //RED
					strip_show();					
				break;
				
				case 1:
					strip_fill(0, 150, 0, 0, NUM_LEDS); //GREEN
					strip_show();
				break;
				
				case 2:
					strip_fill(0, 0, 150, 0, NUM_LEDS); //BLUE
					strip_show();
				break;
				
				case 3:
					strip_fill(150, 0, 150, 0, NUM_LEDS); //PURPLE
					strip_show();
				break;
				
				case 4:
					strip_fill(150, 30, 0, 0, NUM_LEDS); //ORANGE
					strip_show();
				break;
				
				case 5:
					strip_fill(150, 150, 150, 0, NUM_LEDS); //WHITE
					strip_show();
				break;
				
				case 6:
					strip_ASU();	
				break;
				
				case 7:
					strip_fade();
				break;
	
				case 8:
					strip_run();
				break;
				
				case 9:
					strip_rainbow();
				break;
				
				case 10:
					strip_clear(); //turn LEDS off
				break;
				
			}
		}
	}
}


//This interupt service routine starts when strip_show is called
//and produces the correct PWM signal to make the correct color that was specified
//in the strip_fill function. 
void TIM4_IRQHandler()
{
	if(TIM4->SR & TIM_SR_UIF)
	{		
		TIM4->SR &= ~(TIM_SR_UIF);
		
		if(resetFlag != 1)
		{	
			if(neoPixelBuffer[currentLED][currentRGB] == 1 )
			{
				pwm_duty_cicle_neopixels = 57; //creats a high pulse to satisfy the neopixels timing requirement for a 1	
			}
			else
			{
				pwm_duty_cicle_neopixels = 27; //creats a high pulse to satisfy the neopixels timing requirement for a 0
			}
			currentRGB++;
			TIM4->CCR4 = pwm_duty_cicle_neopixels;

			if(currentRGB>23) //the neopixel needs 24 bits for the color. 8 for red, 8 for green, 8 for blue
			{									//once it has completed an etire color, it will move on to the next LED
				currentRGB = 0;
				currentLED++;
			}
		}		
		resetFlag = 0;
		TIM4->ARR = 700;
	}
}


//this function "maps" the values from a given varibale, the range of values that that variable has, 
//and will return a new proportional value in the range of the users choice
uint16_t map(uint16_t var, uint16_t var_low, uint16_t var_high, uint16_t low_thresh, uint16_t high_thresh)
{
	return (var -  var_low)*(high_thresh - low_thresh)/(var_high - var_low) + low_thresh;
}

void init_PWM_TIM4(void)
{	
	//init timer and alternate function 
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	//init the ARR 
	TIM4->CR1 |= TIM_CR1_ARPE;
	
	//init interrupts
	TIM4->CR1 |= TIM_CR1_URS;
	
	//update interrupt enable
	TIM4->DIER |= TIM_DIER_UIE; 
	
	//tell the NVIC to listen to interupts on the timer 4 IRQ number
	NVIC_EnableIRQ(TIM4_IRQn);
	
	//enable PWM mode 1 and the preloader 
	TIM4->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE;

	//PWM DC = CRR4/ARR
	//enable duty cycle
	TIM4->PSC = 0;
	TIM4->ARR = 700; //this value must generate a frequency no larger than the reset time of the neopixel(see datasheet)
	TIM4->CCR4 = pwm_duty_cicle_neopixels;
	
	//enable the update generation
	TIM4->EGR |= TIM_EGR_UG;
}

//The strip_fill funcion lights up the LEDs in the desired range.
//For example, if you want the first 3 LEDs to be green, you would
//say strip_fill(0, 255, 0, 0, 3).
void strip_fill(uint32_t R, uint32_t G, uint32_t B, int start, int END)
{
  uint32_t grb = B | (R << 8) | (G << 16);
  
	if(END > NUM_LEDS) //if the user accidently inputs an end value that is higher than the 
	{									 //NUM_LEDS macro, then END will be set to NUM_LEDS
		END = NUM_LEDS;
	}
	
  for(int j = start; j < END; j++)
  {
    for(int i = 0; i < 24; i++)
    {
      if( ((grb >> i) & 0x01) == 1) 
      {
        neoPixelBuffer[j][23-i] = 1;
      }
      else
      {
        neoPixelBuffer[j][23-i] = 0;
      }
    }
  }
}

void strip_show()
{
	resetFlag = 1; //set reset flag so the function inside the interrupt isn't used 
	
	TIM4->ARR = 6000; //set the ARR to a value that satisfies the data sheet of a low time of at most 50uS(+50us = reset command)
	TIM4->CCR4 = 0; //set the pwm signal to low 
	
	TIM4->CCER |= TIM_CCER_CC4E; //turn on pwm channel 4
	TIM4->CR1 |= TIM_CR1_CEN; //turn on timer
	
	while(currentLED < NUM_LEDS); //wait until all the LEDS are set
	
	TIM4->CCER &= ~(TIM_CCER_CC4E); //turn off channel 4
	TIM4->CR1 &= ~(TIM_CR1_CEN); //turn off timer

	currentLED = 0;
	currentRGB = 0;
	
}

//lights up the first half of LEDs red, and the other half yellow
void strip_ASU()
{
	strip_clear();
	strip_fill(150, 0, 0, 0, NUM_LEDS/2);
	strip_fill(150, 75, 0, NUM_LEDS/2, NUM_LEDS);
	strip_show();
}

//fades the LEDs from color to color
void strip_fade()
{
	static uint8_t currentColor = 2;
	static uint8_t currentColorCount = 0;
	
	if(currentColor > 150)
	{
		currentColor = 150;
		currentColorCount++;
	}
	else if(currentColor < 2)
	{
		currentColor = 2;
		currentColorCount++;
	}
	
	if(currentColorCount > 5)
	{
		currentColorCount = 0;
	}
	
	if(currentColorCount == 0)
	{
		currentColor+=2;
		strip_fill(150, currentColor, 0, 0, NUM_LEDS);
	}
	else if(currentColorCount == 1)
	{
		currentColor-=2;
		strip_fill(currentColor, 150, 0, 0, NUM_LEDS);
	}
	else if(currentColorCount == 2)
	{
		currentColor+=2;
		strip_fill(0, 150, currentColor, 0, NUM_LEDS);
	}
	else if(currentColorCount == 3)
	{
		currentColor-=2;
		strip_fill(0, currentColor, 150, 0, NUM_LEDS);
	}
	else if(currentColorCount == 4)
	{
		currentColor+=2;
		strip_fill(currentColor, 0, 150, 0, NUM_LEDS);
	}
	else if(currentColorCount == 5)
	{
		currentColor-=2;
		strip_fill(150, 0, currentColor, 0, NUM_LEDS);
	}
	strip_show();
	
}

//changes a single LED at a time on one half while changing the other half to a different color at a time.
//this creates what looks like to be a "running" annimation of the LEDs
void strip_run()
{
	static uint8_t runCount = 0;
	static uint8_t colorCount = 0;	
	
	runCount++;

	if(runCount > (NUM_LEDS/2))
	{
		runCount = 1;
		colorCount++;
	}
	if(colorCount > 2)
		colorCount = 0;
	
	if(colorCount == 0)
	{
		strip_fill(75, 0, 150, NUM_LEDS-runCount, NUM_LEDS);
		strip_fill(0, 150, 0, 0, runCount);
	}
	else if(colorCount == 1)
	{
		strip_fill(150, 75, 0, NUM_LEDS-runCount, NUM_LEDS);
		strip_fill(0, 75, 150, 0, runCount);
	}
	else if(colorCount == 2)
	{
		strip_fill(150, 150, 150, NUM_LEDS-runCount, NUM_LEDS);
		strip_fill(150, 0, 0, 0, runCount);
	}
		
	strip_show();

}

//creates a plain rainbow effect to the LEDs (this function is proportional to the amount of LEDs present,
//therefor you can change the amount of leds and not have to worry about messing up this animation)
void strip_rainbow()
{
	const uint8_t colorOffset = (NUM_LEDS/9)+1;
	
	static uint8_t led_start = 0;
	static uint8_t led_finish = 0;
	
	led_start = 0;
	led_finish = 0;
	
	led_finish = led_finish + colorOffset;
	
	strip_fill(150,0,0,led_start,led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(150,37,0,led_start, led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(150, 112, 0, led_start, led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(0,150,0,led_start,led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(0,75,150,led_start, led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(0, 0, 150, led_start, led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(75,0,150,led_start,led_finish);
	
	led_start = led_finish;
	led_finish = led_finish + colorOffset;
	
	strip_fill(150,0,150,led_start, led_finish);
	
	strip_show();
}


//sets the LEDs to no color. Be careful, even though the LEDs are off, the neopixels still draw current
//because they are doing work to send zeros across each other.
void strip_clear()
{
	strip_fill(0,0,0,0,NUM_LEDS);
}

