//===================================================================================================================
//																					CODE FOR THE REMOTE
//===================================================================================================================


#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "GPIO_Driver.h"
#include "NRF_Driver.h"
#include "delay.h"
#include "LCD_Driver.h"
#include "RTC_Driver.h"
#include "EEPROM_Driver.h"

#define RX_PIPE 1
#define ADDR_HIGH 0xAABBCCDD
#define ADDR_LOW 0xEE
#define EN_D_PL_W 1
#define EN_AUTO_ACK 1
#define PL_W 3
#define CHANNEL 0x7B

#define EEPROM_ADDRESS 0xA0

#define PASSWORD_ADDRESS_HI 			0x00
#define PASSWORD_ADDRESS_LO 			0x01


//Each part has two qualities: a name, and how many minutes that part has spent riding.
#define PART_NAME_ADDRESS_HI			0x01
#define PART_NAME_ADDRESS_LO 			0x00

#define PART_TIME_ADDRESS_HI			0x01
#define	PART_TIME_ADDRESS_LO			0x0B
				
//The LED_ANIMATION_ADDR_HI and LO addresses store the LED animation value that was
//picked in the handleLCD_neopixel_scene() function so that the user doesn't
//have to set the LED animation he wants every time he turns on the remote.
#define LED_ANIMATION_ADDR_HI			0x04
#define LED_ANIMATION_ADDR_LO			0x00


void init_ADC(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
uint16_t map(uint16_t var, uint16_t var_low, uint16_t var_high, uint16_t low_thresh, uint16_t high_thresh);

void handleLCD_lock_scene(void);
void handleLCD_menu_scene(void);
void handleLCD_ride_scene(void);
void handleLCD_set_time_scene(void);
void handleLCD_set_password_scene(void);
void handleLCD_add_part_scene(uint8_t part_number);
void handleLCD_display_parts_scene(void);
void handleLCD_neopixel_scene(void);

void load_eeprom(void);
void load_parts_in_use(void);

volatile static uint8_t toggle_button_val = 0;
volatile static uint8_t brake_button_val = 0;

uint16_t static adc_data[2] = {0, 0}; //adc[0] represents the voltage divider values
																			//adc[1] represents the analog slider values

//the parts_in_use[] global array is used in the following functions: handleLCD_ride_scene(),
//handleLCD_add_part_scene(uint8_t part_number), and load_parts_in_use(). The purpose of this gloabal
//array is to keep track of which parts are in use in order to save time reading and writing to
//the unused part addresses in the EEPROM. The array contains either a 0 or a 1, depending on if
//the part (the index) is in use.
uint8_t	static parts_in_use[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
																																			
	
int main() 
{		
	uint8_t static hourglass[8] =
	{
		0x1F,	//0b00011111,
		0x11,	//0b00010001,
		0x0A,	//0b00001010,
		0x04,	//0b00000100,
		0x0A,	//0b00001010,
		0x11,	//0b00010001,
		0x1F,	//0b00011111,
		0x00	//0b00000000
	};

	uint8_t static battery[8] =
	{
		0x04,	//0b00000100,
		0x1F,	//0b00011111,
		0x11,	//0b00010001,
		0x11,	//0b00010001,
		0x1F,	//0b00011111,
		0x1F,	//0b00011111,
		0x1F,	//0b00011111,
		0x1F,	//0b00011111
	};

	uint8_t static locked_lock[8] =
	{
		0x00,	//0b00000000,
		0x0E,	//0b00001110,
		0x11,	//0b00010001,
		0x11,	//0b00010001,
		0x1F,	//0b00011111,
		0x1B,	//0b00011011,
		0x1B,	//0b00011011,
		0x1F,	//0b00011111
	};

	uint8_t static unlocked_lock[8] =
	{
		0x0E,	//0b00001110,
		0x11,	//0b00010001,
		0x01,	//0b00000001,
		0x01,	//0b00000001,
		0x1F,	//0b00011111,
		0x1B,	//0b00011011,
		0x1B,	//0b00011011,
		0x1F,	//0b00011111
	};
	
	
	//==========================================
	GPIO_TYPE toggle_button;
	toggle_button.port = PORTA;
	toggle_button.pin = 0;
	toggle_button.mode = INPUT;
	toggle_button.speed = SPEED_50MHZ;
	//==========================================
	config_gpio_interrupt(toggle_button.port, toggle_button.pin, RISING_EDGE);
	enable_gpio_interrupt(toggle_button.pin, EXTI0_IRQn);
	
	
	//==========================================
	GPIO_TYPE brake_button;
	brake_button.port = PORTA;
	brake_button.pin = 1;
	brake_button.mode = INPUT;
	brake_button.speed = SPEED_50MHZ;
	//==========================================
	config_gpio_interrupt(brake_button.port, brake_button.pin, RISING_FALLING_EDGE);
	enable_gpio_interrupt(brake_button.pin, EXTI1_IRQn);
	
	
	//==========================================
	GPIO_TYPE potentiometer;
	potentiometer.port = PORTA;
	potentiometer.pin = 2;
	potentiometer.mode = INPUT;
	potentiometer.mode_type = INPUT_ANALOG;
	potentiometer.speed = SPEED_50MHZ;
	//==========================================

	init_delay();
	init_ADC();
	eeprom_init();
	lcd_init(PORTA, 8, PORTA, 9, PORTA, 10, PORTA, 11, PORTB, 7, PORTB, 8, PORTB, 9);
	rtc_init();
	
	nrf_init_pins(PORTA, 12, PORTB, 15);
	nrf_init_transmiter(ADDR_HIGH, ADDR_LOW, CHANNEL, EN_AUTO_ACK, EN_D_PL_W);
	nrf_cmd_act_as_TX();

	lcd_create_character(1, hourglass);
	lcd_create_character(2, battery);
	lcd_create_character(3, locked_lock);
	lcd_create_character(4, unlocked_lock);

	//load_eeprom();     //this function should only be called when the user wants to initiate or reset the data related to the parts 
	load_parts_in_use();
		
	handleLCD_lock_scene();

	while(1)
	{
			handleLCD_menu_scene();
	}
}



//The EXTI0_IRQHandler interupt service routine changes the value of toggle_button_val
//from 1 to 0 or from 0 to 1 when a rising edge is detected on the toggle_button 
void EXTI0_IRQHandler()
{
	EXTI->PR |= (1<<0); //clears interrupt
	
	if(toggle_button_val == 0)
		toggle_button_val = 1;
	else
		toggle_button_val = 0;

}

//The EXTI1_IRQHandler interupt service routine changes the value of toggle_brake_button_val
//from 1 to 0 or from 0 to 1 when a rising or falling edge is detected on the brake_button 
void EXTI1_IRQHandler()
{
	EXTI->PR |= (1<<1); //clears interrupt
	
	if(brake_button_val == 0)
		brake_button_val = 1;
	else
		brake_button_val = 0;
}

//The map function takes a variable(var) and it's lowest and highest possible 
//values(var_low and var_high) and returns a new value that correctly maps to 
//the users defined lowest and highest values (low_thresh and high_thresh).
uint16_t map(uint16_t var, uint16_t var_low, uint16_t var_high, uint16_t low_thresh, uint16_t high_thresh)
{
	return (var -  var_low)*(high_thresh - low_thresh)/(var_high - var_low) + low_thresh;
}


//The init_ADC function writes to the registers relating to the STM32's internal
//analog to digital converter.
void init_ADC(void)
{
	
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6; //sets the prescaler on the ADC clock to 6
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; //enables the clock for the DMA
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_AFIOEN; //enable clock for ADC1 and Alternate IO clock
	
	ADC1->CR2 |= ADC_CR2_DMA | ADC_CR2_CONT; //enables DMA for the ADC and enables continuous mode
	ADC1->CR1 |= ADC_CR1_SCAN; //enables SCAN mode (DMA only);
	
	DMA1_Channel1->CPAR |= (uint32_t)(&(ADC1->DR)); //tells the DMA to get data from the ADC1_DR reg
	DMA1_Channel1->CMAR |= (uint32_t)adc_data; //tells the DMA to send the data to ADC_samples
	DMA1_Channel1->CNDTR |= 2; //tells the DMA how much data to send (size of the ADC_samples)
	DMA1_Channel1->CCR |= DMA_CCR1_CIRC | DMA_CCR1_MINC; //sets the DMA to circular mode and implements memory incremation
	DMA1_Channel1->CCR |= DMA_CCR1_PSIZE_0; //tells the ADC that the peripheral has a size of uint16_t
	DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0; //tells the ADC that ADC_samples has a size of uint16_t
	DMA1_Channel1->CCR |= DMA_CCR1_EN; //enables DMA
	
	ADC1->SMPR2 |= ADC_SMPR2_SMP2_0 | ADC_SMPR2_SMP2_1 | ADC_SMPR2_SMP2_2 | //sets channel 2 sampling rate 
								 ADC_SMPR2_SMP8_0 | ADC_SMPR2_SMP8_1 | ADC_SMPR2_SMP8_2; //sets channel 8 sampling rate
	ADC1->SQR1 |= ADC_SQR1_L_0; //tells the ADC to do 2 conversions
	ADC1->SQR3 |= ADC_SQR3_SQ1_1; //sets the first conversion in the sequence equal to channel 2
	ADC1->SQR3 |= ADC_SQR3_SQ2_3; //sets the second conversion in the sequence equal to channel 8
	
	
	ADC1->CR2 |= ADC_CR2_ADON; //wakes up the ADC 
	delayMs(1); //satisy the t(stab) for the ADC when it's waking up
	ADC1->CR2 |= ADC_CR2_ADON; //turns on the ADC
	delayMs(1); //satisy the t(stab)
	
	ADC1->CR2 |= ADC_CR2_CAL; //starts a calibration on the ADC
	while(ADC1->CR2 == ADC_CR2_CAL); //wait until the CAL bit = 0 (this means the ADC made a sucessful converstion)
}


//The handleLCD_lock_scene function is called before main and in the handleLCD_set_password_scene
//function. It's purpose is to halt the user from entering any other scene until the correct password
//has been inputted. It's other purpose is to prevent the user from being able to change the password 
//in the handleLCD_set_password_scene function without firstly giving the current password.
void handleLCD_lock_scene(void)
{
	uint8_t	 actual_password[4];
	
	//These 4 lines below load the actual_password[]  array with the password
	//that is currently in the EEPROM at addresses PASSWORD_ADDRESS_HI and PASSWORD_ADDRESS_LO.
	//Also, the reason why PASSWORD_ADDRESS_LO is being incremented by 1 each time the actual_password
	//index increases by 1 is because each digit of the password is stored in a seperate address.
	//For example, if the password is '1742', then '1' is stored in PASSWORD_ADDRESS_LO,
	//'7' is stored in PASSWORD_ADDRESS_LO+1 etc. The reason for this is because the addresses
	//can only hold numbers less than 2^8 (256) and I wanted the password to be 4 digits long.
	//Holding the individual digits in an array also helps with the logic and flow of this function.
	actual_password[0] = eeprom_read(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO);
	actual_password[1] = eeprom_read(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO+1);
	actual_password[2] = eeprom_read(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO+2);
	actual_password[3] = eeprom_read(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO+3);
	
	//The attempted_password[] array will hold the temporary attempted password and at the end of the
	//4th digit, will compare each element with the actual_password[] array.
	uint16_t attempted_password[4] = {0, 0, 0, 0};
	uint8_t isPassword = 0;
	
	//Unfortunatly, I did not create a function in the LCD_Driver library to print char arrays, so
	//you must print the characters individualy, which I know is lame. But im lazy.
	lcd_set_cursor(0,0);
	lcd_write_char('P');
	lcd_write_char('A');
	lcd_write_char('S');
	lcd_write_char('S');
	lcd_write_char('W');
	lcd_write_char('O');
	lcd_write_char('R');
	lcd_write_char('D');
  lcd_write_char(':');
	lcd_write_char('*');
	lcd_write_char('*');
	lcd_write_char('*');
	lcd_write_char('*');
	
	lcd_set_cursor(0,14);
	lcd_write_char(3); //display the locked_lock graphic that was created in main
	
	
	//The do while loop will repeat until the isPassword variable equals 4. This variable increases when
	//a digit in the attempted_password[4] array equals actual_password[4] array (this is handled in lines
	//322-327). 
	do
	{
		//this for loop ends when the user succesfully inputs 4 digits.
		for(uint8_t k = 0; k < 4; k++)
		{
			while(toggle_button_val != 1)
			{
				lcd_set_cursor(0,9+k);
				attempted_password[k] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 10));
				lcd_write_number(attempted_password[k]);
			}
			lcd_set_cursor(0,9+k);
			lcd_write_char('*');
			toggle_button_val = 0;	
		}
			
		for(int p = 0; p < 4; p++)
		{
			if(attempted_password[p] == actual_password[p])
			{
				isPassword++;
			}
		}

		//if the password was not correct, everything in this if stated will be executed.
		if(isPassword != 4)
		{
			isPassword = 0;
			//this for loops produces a locking animation 
			for(int j = 0; j < 6; j++) 
			{
				lcd_set_cursor(0,14);
				if(j == 1 || j == 3 || j == 5)
					lcd_write_char(3); //locked character
				else
					lcd_write_char(' ');
				delayMs(200);
			}
			lcd_set_cursor(0,9);
			lcd_write_char('*');
			lcd_write_char('*');
			lcd_write_char('*');
			lcd_write_char('*');
		}
	}while(isPassword != 4);	
	
	lcd_set_cursor(0,14);
	lcd_write_char(4); //unlocked character 
	delayMs(1000);
	
	lcd_clear(0, 0, 16);
	
}


//The handleLCD_menu_scene function displays all of the scenes that the
//user can choose from and when the toggle_button is pressed, executes the users chosen
//scene. These scenes are the handleLCD_ride_scene(), handleLCD_set_time_scene(),
//handleLCD_set_password_scene(), handleLCD_display_parts_scene(), and handleLCD_neopixel_scene().
void handleLCD_menu_scene(void)
{
	uint8_t menu_selection;
	
	do
	{
		 //The menu_selection varible has a range of [0,5) and is changed by adc_data[1], which
		 //is the data that comes from the analog slider.
		 menu_selection = (uint8_t)map(adc_data[1], 0, 4096, 0, 5);
		
		//NOTE: Make this a switch case statement instead of 4 if statements. 
		if(menu_selection == 0)
		{
			lcd_set_cursor(0,0);
			lcd_write_char('R');
			lcd_write_char('I');
			lcd_write_char('D');
			lcd_write_char('E');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
		}
		if(menu_selection == 1)
		{
			lcd_set_cursor(0,0);
			lcd_write_char('S');
			lcd_write_char('E');
			lcd_write_char('T');
			lcd_write_char(' ');
			lcd_write_char('T');
			lcd_write_char('I');
			lcd_write_char('M');
			lcd_write_char('E');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
		}
		if(menu_selection == 2)
		{
			lcd_set_cursor(0,0);
			lcd_write_char('S');
			lcd_write_char('E');
			lcd_write_char('T');
			lcd_write_char(' ');
			lcd_write_char('P');
			lcd_write_char('A');
			lcd_write_char('S');
			lcd_write_char('S');		
			lcd_write_char('W');
			lcd_write_char('O');
			lcd_write_char('R');
			lcd_write_char('D');	
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');			
		}
		if(menu_selection == 3)
		{
			lcd_set_cursor(0,0);
			lcd_write_char('D');
			lcd_write_char('I');
			lcd_write_char('S');
			lcd_write_char('P');
			lcd_write_char('L');
			lcd_write_char('A');
			lcd_write_char('Y');
			lcd_write_char(' ');		
			lcd_write_char('P');
			lcd_write_char('A');
			lcd_write_char('R');
			lcd_write_char('T');	
			lcd_write_char('S');
			lcd_write_char(' ');
			lcd_write_char(' ');
			lcd_write_char(' ');			
		}
		if(menu_selection == 4)
		{
			lcd_set_cursor(0,0);
			lcd_write_char('S');
			lcd_write_char('E');
			lcd_write_char('L');
			lcd_write_char('E');
			lcd_write_char('C');
			lcd_write_char('T');
			lcd_write_char(' ');
			lcd_write_char('L');		
			lcd_write_char('E');
			lcd_write_char('D');
			lcd_write_char(' ');
			lcd_write_char(' ');	
			lcd_write_char(' ');
			lcd_write_char(' ');		
		}
	}while(toggle_button_val != 1);
	
	toggle_button_val = 0;
	
	switch(menu_selection)
	{
		case 0:
			handleLCD_ride_scene();
		break;
			
		case 1:
			handleLCD_set_time_scene(); 
		break;
			
		case 2:
			handleLCD_set_password_scene();
		break;
		
		case 3:
			handleLCD_display_parts_scene();
		break;
		
		case 4:
			handleLCD_neopixel_scene();
		break;
	}
}


//The handleLCD_set_password_scene function allows the user to change the
//password after inputing the correct current password. The new password
//is loaded into EEPROM so the system remebers after a power off reset.
void handleLCD_set_password_scene()
{
	handleLCD_lock_scene(); //Make the user correctly input the current password
	
	uint8_t password[4] = {0, 0, 0, 0};
	
	lcd_set_cursor(0,0);
	lcd_write_char('N');
	lcd_write_char('E');
	lcd_write_char('W');
	lcd_write_char(' ');
	lcd_write_char('P');
	lcd_write_char('W');
	lcd_write_char('O');
	lcd_write_char('R');
	lcd_write_char('D');
	lcd_write_char('>');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	
	for(uint8_t i = 0; i < 4; i++)
	{
		while(toggle_button_val != 1)
		{
			lcd_set_cursor(0,10+i);
			password[i] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 10));
			lcd_write_number(password[i]);
		}
		toggle_button_val = 0;
		
	}
	
	eeprom_write(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO, password[0]);
	eeprom_write(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO+1, password[1]);
	eeprom_write(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO+2, password[2]);
	eeprom_write(EEPROM_ADDRESS, PASSWORD_ADDRESS_HI, PASSWORD_ADDRESS_LO+3, password[3]);
	
	lcd_clear(0, 0, 16);
		
}


//The handleLCD_set_time_scene function allows the user to change the time that is displayed
//in the handleLCD_ride_scene function.
void handleLCD_set_time_scene()
{
	uint8_t rtc_time[4] = {0, 0, 0, 0};
	uint8_t hours = 0;
	uint8_t minutes = 0;
	
	lcd_set_cursor(0,0);
	lcd_write_char('N');
	lcd_write_char('E');
	lcd_write_char('W');
	lcd_write_char(' ');
	lcd_write_char('T');
	lcd_write_char('I');
	lcd_write_char('M');
	lcd_write_char('E');
	lcd_write_char('>');
	
	//The reason I did not contain all of these while loops in a foor loop
	//is because the input changes depending on what the user chooses. For
	//example, the range of the first digit is 3, while the range of the second digit is 10,
	//and the range of the third digit is 6. I could make a for loop and incorparate a lot of
	//if statements, but I just decided to handle one digit at a time and have seperate while
	//loops for each.
	
	while(toggle_button_val != 1) //wait for the toggle button to be pressed 
	{
		lcd_set_cursor(0,9);
		rtc_time[3] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 3));
		lcd_write_number(rtc_time[3]);
		lcd_write_char('*');
		lcd_write_char(':');
		lcd_write_char('*');
		lcd_write_char('*');
	}
	
	toggle_button_val = 0; //reset toggle button value
	
	while(toggle_button_val != 1) //wait for the toggle button to be pressed 
	{
		lcd_set_cursor(0,10);
		rtc_time[2] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 10));
		
		if(rtc_time[3] == 2) //this is a case where the first digit is 2. If it is, then the third digit 
			                   // cannot exceed 3 (military time -> 00:00 - 23:59)
		{
			if(rtc_time[2] > 3)
			{
				rtc_time[2] = 3;
			}
		}
		
		lcd_write_number(rtc_time[2]);
		lcd_write_char(':');
		lcd_write_char('*');
		lcd_write_char('*');
	}
	
	toggle_button_val = 0; //reset toggle button value
		
	while(toggle_button_val != 1) //wait for the toggle button to be pressed 
	{
		lcd_set_cursor(0,12);
		rtc_time[1] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 6));
		lcd_write_number(rtc_time[1]);
		lcd_write_char('*');
	}
		
	toggle_button_val = 0; //reset toggle button value
	
	while(toggle_button_val != 1) //wait for the toggle button to be pressed 
	{
		lcd_set_cursor(0,13);
		rtc_time[0] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 10));
		lcd_write_number(rtc_time[0]);
	}
	
	toggle_button_val = 0; //reset toggle button value
	
	//turns the indivual digits into numbers that can be inputed in rtc_set_time
	hours = (rtc_time[3] * 10) + rtc_time[2];  
	minutes = (rtc_time[1] * 10) + rtc_time[0];
	
	rtc_set_time(hours, minutes, 0);
	
	lcd_clear(0, 0, 16);
	
}


//The handleLCD_ride_scene function allows the user to send input from the analog slider
//and the brake button to the skateboard. While in this function, the user can also see the 
//time of day and the current battery percentage of the remote. Accessing this function also
//sends the last loaded LED animation in EEPROM to the skateboard. When the user exists this function,
//then the time (in minutes) that passed during the time in this scene get added to the parts in use
//(more info of this can be found in the functions handleLCD_display_parts_scene and handleLCD_add_parts_scene
void handleLCD_ride_scene(void)
{
	uint8_t batteryLevel = 0;
	uint8_t rtc_data[3];
	uint8_t nrf_data[4];
	uint8_t compare_minute = 0;
	uint16_t minute_counter = 0;	//This variable keeps track of the minutes that pass during the time spent in this function 
	
	lcd_set_cursor(1,0);
	lcd_write_char(2); //print the hourglass graphic made in main
	lcd_write_char('>');
	
	lcd_set_cursor(0,0);
	lcd_write_char(1); //print the battery graphic made in main
	lcd_write_char('>');
	
	lcd_set_cursor(1, 4);
	lcd_write_char('%');	
	
	reset_millis(); //These functions are described in the delay.h header file
	
	nrf_data[2] = eeprom_read(EEPROM_ADDRESS, LED_ANIMATION_ADDR_HI, LED_ANIMATION_ADDR_LO);
	
	do
	{ 
		
		//TX MODE
		nrf_data[0] += 1;   //This value is dumby data that is constantly changing in order for the 
												//NRF24L01 module on the skateboard side to constantly set the 
												//RX_DR interrupt. I do this because of a feature on the skateboard
												//that detects when the RX_DR hasn't been set for a certain amount
		                    //of time, which indicates that the remote lost connection and slows
		                    //down the skateboard
		nrf_data[1] =	(uint8_t)(map(adc_data[1], 0, 4096, 0, 255));
		nrf_data[3] = brake_button_val;
		batteryLevel = (uint8_t)(map(adc_data[0], 2600, 3800, 0, 100));
		
		if(nrf_data[0] > 250)
			nrf_data[0] = 0;
		
		if(nrf_send(nrf_data, 32))
		{
				//I had an LED toggling in here for debugging purposes initially
		}

		//the play_millis function is described in the delay.h header, but basically 
		//this if statement executes every (5000 * 1000)Us, which is 5 seconds. I
		//implemented this so that the data that controls the skateboard is sent at a much faster rate
		//than what is inside this if statement, which is basically just updating the LCD screen with the time 
		//and the battery percentage of the remote. I know now that an RTOS would be much more appropriate 
		//for this type of behavior, so I will be working on that for version 2.0
		if(play_millis(5000*1000) == 0)
		{	
			compare_minute = rtc_get_minutes();
			
			if(rtc_data[1] != compare_minute) //the rtc_data[1] is changed a few lines down,
					minute_counter++;							//so what this if statement does is it checks to see if the 
																				//"current" minute (compare_minute) is different to the value 
																				//in rtc_data[1]. If it is, then it knows that the minute has 
																				//just increased, therefore I increment the minute_counter.
																				//It would be better if the RTC module had data like this that
																				//I could access, but unfortunately I had to implement this myself.
				
			
			lcd_set_cursor(1, 2);

			if(batteryLevel < 10) 
				lcd_write_char('0');
			lcd_write_number(batteryLevel);
			
			rtc_data[0] =	rtc_get_seconds();
			rtc_data[1] = rtc_get_minutes();
			rtc_data[2] = rtc_get_hours();
		
			lcd_set_cursor(0, 2);
			
			if(rtc_data[2] < 10)
				lcd_write_char('0');
			
			lcd_write_number(rtc_data[2]);
			lcd_set_cursor(0, 4);
			lcd_write_char(':');
			
			if(rtc_data[1] < 10)
				lcd_write_char('0');
			
			lcd_write_number(rtc_data[1]);
			lcd_set_cursor(0,7);
		}		
		
	}while(toggle_button_val != 1);
	
	toggle_button_val = 0;
	
	minute_counter--; //this eliminates the first compare (rtc_data[1] != compare_minute) because the first 
										//compare will almost always have a different value, resulting in an incrementaion of 
										//minute_counter. However, this does not indicate a minute has past, therefor
										//I subtract one from the minute_counter variable.
	
	
	uint16_t part_time_spent_new = 0;
	uint16_t part_time_spent_current = 0; 
	
	//This for loop saves the minutes to the parts that are only in use 
	for(uint8_t k = 0; k < 16; k++) 
	{
		if(parts_in_use[k] == 1) //check to see if the part is in use 
		{
			
			//because the EEPROM address are 1 byte each and I want the number of minutes to be as big
			//as 9999, then I have to store the hi and low bits of the minutes in seperate registers.
			//If I want to access the entire number of minutes, then I have to take the hi part of the number,
			//shift it left by 8, then do a |= on the low part of the number.
			part_time_spent_current = eeprom_read(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*k)+(PART_TIME_ADDRESS_LO)));
			part_time_spent_current = (uint16_t)(part_time_spent_current<<8);
			part_time_spent_current |= eeprom_read(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*k)+(PART_TIME_ADDRESS_LO+1)));
					
			part_time_spent_new = part_time_spent_current + minute_counter;
		
			eeprom_write(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*k)+(PART_TIME_ADDRESS_LO)), (part_time_spent_new>>8)); //stores the high value 
			eeprom_write(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*k)+(PART_TIME_ADDRESS_LO+1)), (part_time_spent_new&0x00FF));//stores the low value
			
		}
	}
	
	lcd_clear(0, 0, 16);
	lcd_clear(1, 0, 16);
}


//The handleLCD_display_parts_scene function displays 16 different part slots that the user
//can choose from. Each slot can have a part name and the time (in minutes) that that part has
//been ridden for. If the toggle_button is pressed while on the selected part, then the
//handleLCD_add_parts_scene function will be called. After the add_parts_scene function, then the 
//user is taken back to handleLCD_display_parts_scene and the new part and new time are read from
//EEPROM and displayed on the LCD screen. The user can exit the function when the toggle_button
//is pressed when the LCD displays "Exit".
void handleLCD_display_parts_scene()
{
	uint16_t part_time_spent = 0;
	uint8_t display_parts_menu_selection;
	uint8_t part_name_from_eeprom;
	uint8_t empty_part_detection = 0;
	
	lcd_clear(0, 0, 16);
	
	do
	{
		display_parts_menu_selection = (uint8_t)map(adc_data[1], 0, 4096, 0, 17);
			
		if(display_parts_menu_selection == 16) //display the exit scene
		{
			lcd_clear(1,0,16);
			lcd_set_cursor(0,4);
			lcd_write_char(' ');	
			lcd_write_char(' ');	
			lcd_write_char('E');
			lcd_write_char('X');
			lcd_write_char('I');
			lcd_write_char('T');
			lcd_write_char(' ');	
			lcd_write_char(' ');	
		}
		else
		{
			lcd_set_cursor(0,4);
			lcd_write_char('P');
			lcd_write_char('A');
			lcd_write_char('R');
			lcd_write_char('T');
			lcd_write_char(' ');
			
			if(display_parts_menu_selection < 9)
				lcd_write_char('0');
			lcd_write_number(display_parts_menu_selection+1);
			
			lcd_set_cursor(1,0);
			
			//This for loops displays the specified part from the user (it changes depending on the 
			//analog slider value which is stored in the variable display_parts_menu_selection)
			for(uint8_t r = 0; r < 11; r++)
			{
				//the part_name_from_eeprom equals the correct ASCII value stored in EEPROM 
				part_name_from_eeprom = eeprom_read(EEPROM_ADDRESS, PART_NAME_ADDRESS_HI, ((16*display_parts_menu_selection)+(PART_NAME_ADDRESS_LO+r)));
				
				//if the part_name_from_eeprom equals 32, then the character equals a space ' ' and if 
				//all of the ASCII values in the part are spaces, then there is no part in that slot.
				if(part_name_from_eeprom == 32)
				{
					empty_part_detection++;
				}
				lcd_write_char(part_name_from_eeprom);
			}
			if(empty_part_detection != 11) 
			{
				//get the hi and lo bits from the part time address and put them into one 16 bit variable
				part_time_spent = eeprom_read(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*display_parts_menu_selection)+(PART_TIME_ADDRESS_LO)));
				part_time_spent = (uint16_t)(part_time_spent<<8);
				part_time_spent |= eeprom_read(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*display_parts_menu_selection)+(PART_TIME_ADDRESS_LO+1)));
			
				lcd_set_cursor(1,12);
				if(part_time_spent < 10)
				{
					lcd_write_char(' ');
					lcd_write_char(' ');
					lcd_write_char(' ');
					lcd_write_number(part_time_spent);
				}
				else if(part_time_spent < 100)
				{
					lcd_write_char(' ');
					lcd_write_char(' ');
					lcd_write_number(part_time_spent);
				}
				else if(part_time_spent < 1000)
				{
					lcd_write_char(' ');
					lcd_write_number(part_time_spent);
				}
				else
				{
					lcd_write_number(part_time_spent);
				}
			}
			else //if there is no part, than the LCD should not display any time 
			{
				lcd_set_cursor(1,12);
				lcd_write_char(' ');
				lcd_write_char(' ');
				lcd_write_char(' ');
				lcd_write_char(' ');
			}
			empty_part_detection = 0;
		}
			
	}while(toggle_button_val != 1);
	
	toggle_button_val = 0;		

	if(display_parts_menu_selection != 16) //the handleLCD_add_part_scene will only be called when the user
																				 //presses the toggle button while NOT on the exit scene
	{	
		handleLCD_add_part_scene(display_parts_menu_selection);
	}
		
}

//The handleLCD_add_part_scene lets the user choose the name 
//of their part and the time it has already been in use (most of the time it will be 0)
void handleLCD_add_part_scene(uint8_t part_number)
{
	uint8_t	part_name[11];
	uint8_t num_array[4]; //used for the time spent display
	uint16_t time_spent = 0; //value used to put into eeprom
	uint8_t empty_part_detection = 0;
	
	lcd_clear(1,0, 16);
	lcd_set_cursor(0,0);
	lcd_write_char('N');
	lcd_write_char('A');
	lcd_write_char('M');
	lcd_write_char('E');
	lcd_write_char(':');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	lcd_write_char('_');
	
	
	//This foor loops cycles through the naming convention of the part (a total of 11 characters)
	//and stores the ASCII value in the part_name[] array.
	for(uint8_t n = 0; n < 11; n++)
	{
		while(toggle_button_val != 1)
		{
			lcd_set_cursor(0,n+5);
			part_name[n] = (uint8_t)(map(adc_data[1], 0, 4096, 64, 91)); //64-91 is the ascii values for the alphabet in caps
			if(part_name[n] == 64)
			{
				part_name[n] = 32; //change the '@' to a ' ' 
			}
			lcd_write_char(part_name[n]);
		}
		
		if(part_name[n] == 32) //if the part_name[n] is an empty space (32) then increment empty_part_detection
		{											 
			empty_part_detection++;
		}
		toggle_button_val = 0;		
		
	}
	
	
	if(empty_part_detection != 11) //if the part does not equal all empty spaces, then add a time
																 //time to that specific part slot. If not, then this code is skipped
	{
		parts_in_use[part_number] = 1; //loads a 1 in the parts_in_use[part_number] to indicate that
																	 //the slot number of "part_number" is in use.
		
		lcd_clear(1,0, 16);
		lcd_set_cursor(0,0);
		lcd_write_char('T');
		lcd_write_char('I');
		lcd_write_char('M');
		lcd_write_char('E');
		lcd_write_char(' ');
		lcd_write_char('S');
		lcd_write_char('P');
		lcd_write_char('E');
		lcd_write_char('N');
		lcd_write_char('T');
		lcd_write_char(':');
		lcd_write_char('_');
		lcd_write_char('_');
		lcd_write_char('_');
		lcd_write_char('_');
		lcd_write_char(' ');
	
		for(uint8_t t = 0; t < 4; t++)
		{
			while(toggle_button_val != 1)
			{
				lcd_set_cursor(0,t+11);
				num_array[t] = (uint8_t)(map(adc_data[1], 0, 4096, 0, 10)); 
				lcd_write_number(num_array[t]);
			}
			toggle_button_val = 0;		
		}
	}

	empty_part_detection = 0; //resets the empty_part_detection counter
	
	lcd_clear(0, 0, 16);
	
	
	//This for loop loads the EEPROM with the entire name of the specified part in the specified part slot.
	for(uint8_t w = 0; w < 11; w++)
	{
		eeprom_write(EEPROM_ADDRESS, PART_NAME_ADDRESS_HI,((16*part_number)+(PART_NAME_ADDRESS_LO+w)), part_name[w]);
	}
	
	time_spent = (num_array[0] * 1000) + (num_array[1] * 100) + (num_array[2] * 10) + (num_array[3]);
	
	//write the time that was spent on that part into the specified part slot in EEPROM.
	eeprom_write(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*part_number)+(PART_TIME_ADDRESS_LO+1)), (time_spent&0x00FF));
	eeprom_write(EEPROM_ADDRESS, PART_TIME_ADDRESS_HI, ((16*part_number)+(PART_TIME_ADDRESS_LO)), ((time_spent>>8)));
	
	handleLCD_display_parts_scene();
	
}

//The load_eeprom function just loads the EEPROM with ascii values that correlate to empty spaces ' '
//to indicate a fresh slate of part slots with no names or no times.
void load_eeprom()
{
	for(uint8_t i = 0; i < 16; i++)
	{
		for(uint8_t r = 0; r < 11; r++)
		{
			eeprom_write(EEPROM_ADDRESS, PART_NAME_ADDRESS_HI, ((16*i)+(PART_NAME_ADDRESS_LO+r)), 32);
		}
	}
}


//this function is called before main and loads the parts_in_use[] array with either a 1 or a 0,
//depending on if the part (read from EEPROM) has all empty spaces (this would indicate there is 
//no part in that slot and parts_in_use[i] would equal 0. If the EEPROM has a valid ASCII character
//in the part slot, than this indicates a part name, which indicates the part is in use, which means the
//parts_in_use[i] will equal 1.
void load_parts_in_use()
{
	uint8_t check = 0;
	uint8_t empty_part_detection = 0;
	
	//This for loop iterates 16 times, which indicates the size of how many part slots there are
	for(uint8_t i = 0; i < 16; i++)
	{
		//this for loop iterates 11 times, which indicates the size of how big the name of the part can be
		for(uint8_t w = 0; w < 11; w++)
		{
			check = eeprom_read(EEPROM_ADDRESS, PART_NAME_ADDRESS_HI,((16*i)+(PART_NAME_ADDRESS_LO+w)));
			if(check == 32)
				empty_part_detection++;
		}
		if(empty_part_detection != 11)
			parts_in_use[i] = 1;
	}
}


//The handleLCD_neopixel_scene function iterates over 11 different 
//LED animations for the neopixel strip and sends this data to the
//skateboard so it can light up accordingly.
void handleLCD_neopixel_scene()
{
	uint8_t nrf_data[4] = {0, 0, 0, 0}; //even though I am only care about sending nrf_data[2]
																			//which is the value of the selected LED animation,
																		  //I still have to send a packet of 4 because of the way
																			//The skateboard handles the data. 
	lcd_clear(0,0, 16);
	lcd_set_cursor(0,0);
	lcd_write_char('L');
	lcd_write_char('E');
	lcd_write_char('D');
	lcd_write_char(' ');
	lcd_write_char('A');
	lcd_write_char('N');
	lcd_write_char('I');
	lcd_write_char('M');
	lcd_write_char('A');
	lcd_write_char('T');
	lcd_write_char('I');
	lcd_write_char('O');
	lcd_write_char('N');
	lcd_write_char(':');
	
	do
	{ 
		//TX MODE
		nrf_data[2] =	(uint8_t)(map(adc_data[1], 0, 4096, 0, 11));

		lcd_set_cursor(0,14);
		if((nrf_data[2]+1) < 10)
		{
			lcd_write_char('0');
		}
		lcd_write_number(nrf_data[2]+1);
		
		if(nrf_send(nrf_data, 32))
		{
			//I had an LED toggling in here for debugging purposes initially
		}

		
	}while(toggle_button_val != 1);
	
	//Load the EEPROM with the LED animation so that the user does not have to
	//go to this scene every time he rides in order to get his desired animation.
	eeprom_write(EEPROM_ADDRESS, LED_ANIMATION_ADDR_HI, LED_ANIMATION_ADDR_LO, nrf_data[2]);
	
	toggle_button_val = 0;
	
}

