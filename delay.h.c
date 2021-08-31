//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//																			THIS DRIVER USES TIMER 2
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef DELAY_H_
#define DELAY_H_

#include "stm32f10x.h"

//The start_millis function sets the millis_flag and returns the vlaue of ticks_millis
int start_millis(void);

//The reset_millis function resets the millis_flag and sets the value of ticks_millis
//to 0
void reset_millis(void);

//The play_millis function turns the timer on until the specified amount of time has passed (in Us)
//It is basically the start_millis function and reset_millis function put into one function for
//easier use for the user if they know specifically how long they want 
int play_millis(int time);

//The init_delay function initizializes the timer of choice for the use of delay	
void init_delay(void);
	
//The delayUs function stops the mcu for (uS) microseconds 	
void delayUs(int uS);
	

//The delayMs function stops the mcu for (mS) milliseconds
void delayMs(int mS);

//The TIM2_IRQHandler increments the ticks_delay variable every time the timer overflows 
//(which is approximately every microsecond) if the delay
//flag is set (the delay_flag is set when the functions delayUs and delayMs are called) and increments the
//ticks_millis if the millis_flag is set (the millis_flag is set when the play_millis function is called)
void TIM2_IRQHandler(void);
																		
#endif
