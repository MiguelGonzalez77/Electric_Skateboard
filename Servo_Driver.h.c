// This servo driver allows for the use of all the channels on timers 2, 3, and 4
	


#ifndef Servo_Driver
#define Servo_Driver

#include <stdint.h>
#include "stm32f10x.h"

//The servo_init function takes in the number of the timer and channel that
//the user wants to use for the pwm output
void servo_init(uint8_t timer, uint8_t channel);

//The servo_writeMicroseconds function takes in a value in microseconds (1000-2000)
//and outputs the pwm signal accordingly to the given timer and channel. 
//Note that the servo_init function must be called first before this function
//and the timer and channel used in this function must match the timer
//and channel passed into servo_init
void servo_writeMicroseconds(int microseconds, uint8_t timer, uint8_t channel);


#endif
