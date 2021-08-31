#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "GPIO_Driver.h"
#include "delay.h"
#include "LCD_Driver.h"

GPIO_TYPE static D4;
GPIO_TYPE static D5;
GPIO_TYPE static D6;
GPIO_TYPE static D7;
GPIO_TYPE static EN;
GPIO_TYPE static RW;
GPIO_TYPE static RS;

void lcd_init(GPIO_TypeDef *D4port, uint8_t D4pin, GPIO_TypeDef *D5port, uint8_t D5pin, 
							GPIO_TypeDef *D6port, uint8_t D6pin, GPIO_TypeDef *D7port, uint8_t D7pin,
							GPIO_TypeDef *ENport, uint8_t ENpin, GPIO_TypeDef *RWport, uint8_t RWpin,
							GPIO_TypeDef *RSport, uint8_t RSpin)
{
	//==========================================
	D4.port = D4port;
	D4.pin = D4pin;
	D4.mode = OUTPUT;
  D4.mode_type = OUTPUT_GEN_PURPOSE;
	D4.speed = SPEED_2MHZ;
	//==========================================
	
	//==========================================
	D5.port = D5port;
	D5.pin = D5pin;
	D5.mode = OUTPUT;
  D5.mode_type = OUTPUT_GEN_PURPOSE;
	D5.speed = SPEED_2MHZ;
	//==========================================
	
	//==========================================
	D6.port = D6port;
	D6.pin = D6pin;
	D6.mode = OUTPUT;
  D6.mode_type = OUTPUT_GEN_PURPOSE;
	D6.speed = SPEED_2MHZ;
	//==========================================
	
	//==========================================
	D7.port = D7port;
	D7.pin = D7pin;
	D7.mode = OUTPUT;
  D7.mode_type = OUTPUT_GEN_PURPOSE;
	D7.speed = SPEED_2MHZ;
	//==========================================
	
	//==========================================
	EN.port = ENport;
	EN.pin = ENpin;
	EN.mode = OUTPUT;
  EN.mode_type = OUTPUT_GEN_PURPOSE;
	EN.speed = SPEED_2MHZ;
	//==========================================
	
	//==========================================
	RW.port = RWport;
	RW.pin = RWpin;
	RW.mode = OUTPUT;
  RW.mode_type = OUTPUT_GEN_PURPOSE;
	RW.speed = SPEED_2MHZ;
	//==========================================
	
	//==========================================
	RS.port = RSport;
	RS.pin = RSpin;
	RS.mode = OUTPUT;
  RS.mode_type = OUTPUT_GEN_PURPOSE;
	RS.speed = SPEED_2MHZ;
	//==========================================
	
	init_gpio(D4);
	init_gpio(D5);
	init_gpio(D6);
	init_gpio(D7);
	init_gpio(EN);
	init_gpio(RW);
	init_gpio(RS);
	
	//==============init LCD=================
	delayMs(50);
	pin_write(RS.port, RS.pin, LOW);
	pin_write(RW.port, RW.pin, LOW);
	pin_write(D4.port, D4.pin, HIGH);
	pin_write(D5.port, D5.pin, HIGH);
	pin_write(D6.port, D6.pin, LOW);
	pin_write(D7.port, D7.pin, LOW);
	
	lcd_clock();
	
	delayMs(10);
	pin_write(RS.port, RS.pin, LOW);
	pin_write(RW.port, RW.pin, LOW);
	pin_write(D4.port, D4.pin, HIGH);
	pin_write(D5.port, D5.pin, HIGH);
	pin_write(D6.port, D6.pin, LOW);
	pin_write(D7.port, D7.pin, LOW);
	
	lcd_clock();
	
	delayMs(1);
	pin_write(RS.port, RS.pin, LOW);
	pin_write(RW.port, RW.pin, LOW);
	pin_write(D4.port, D4.pin, HIGH);
	pin_write(D5.port, D5.pin, HIGH);
	pin_write(D6.port, D6.pin, LOW);
	pin_write(D7.port, D7.pin, LOW);
	
	lcd_clock();	
	
	//==============init 4 bit mode=================
	pin_write(RS.port, RS.pin, LOW);
	pin_write(RW.port, RW.pin, LOW);
	pin_write(D4.port, D4.pin, LOW);
	pin_write(D5.port, D5.pin, HIGH);
	pin_write(D6.port, D6.pin, LOW);
	pin_write(D7.port, D7.pin, LOW);
	
	lcd_clock();
	
	//==============init 2 line 5X8=================
	lcd_command(FOUR_BIT_MODE);

	//==============display off=================
	lcd_command(DISPLAY_OFF);
	
	//==============clear display=================
	lcd_command(DISPLAY_CLEAR);
	
	//===============entry mode set==========================
	lcd_command(EM_INCREMENT);
	
	//===================display on==============================
	lcd_command(DISPLAY_ON_COFF);
}

void lcd_clock()
{
	pin_write(EN.port, EN.pin, HIGH);
	delayMs(1);
	pin_write(EN.port, EN.pin, LOW);
	delayMs(1);	
}

void lcd_command(uint8_t command)
{
	//high bits
	pin_write(RS.port, RS.pin, LOW);
	pin_write(RW.port, RW.pin, LOW);
	
	pin_write(D4.port, D4.pin, (command>>4)&0x01);
	pin_write(D5.port, D5.pin, (command>>5)&0x01);
	pin_write(D6.port, D6.pin, (command>>6)&0x01);
	pin_write(D7.port, D7.pin, (command>>7)&0x01);
	
	lcd_clock();
	//low bits
	pin_write(RS.port, RS.pin, LOW);
	pin_write(RW.port, RW.pin, LOW);
	
	pin_write(D4.port, D4.pin, (command)&0x01);
	pin_write(D5.port, D5.pin, (command>>1)&0x01);
	pin_write(D6.port, D6.pin, (command>>2)&0x01);
	pin_write(D7.port, D7.pin, (command>>3)&0x01);
	
	lcd_clock();	
}

void lcd_write_char(uint8_t character)
{
	pin_write(RS.port, RS.pin, HIGH);
	pin_write(RW.port, RW.pin, LOW);
	
	pin_write(D4.port, D4.pin, (character>>4)&0x01);
	pin_write(D5.port, D5.pin, (character>>5)&0x01);
	pin_write(D6.port, D6.pin, (character>>6)&0x01);
	pin_write(D7.port, D7.pin, (character>>7)&0x01);
	
	lcd_clock();
	//low bits
	pin_write(RS.port, RS.pin, HIGH);
	pin_write(RW.port, RW.pin, LOW);
	
	pin_write(D4.port, D4.pin, (character)&0x01);
	pin_write(D5.port, D5.pin, (character>>1)&0x01);
	pin_write(D6.port, D6.pin, (character>>2)&0x01);
	pin_write(D7.port, D7.pin, (character>>3)&0x01);
	lcd_clock();	
}

void lcd_write_number(uint16_t number)
{
	uint8_t numberArray[4] = {0, 0, 0, 0};
	uint8_t numOfDigits = 0;
	
	if(number < 10)
	{
			numOfDigits = 1;
			numberArray[0] = (uint8_t)number;
	}
	else if(number < 100)
	{
		numOfDigits = 2;
		numberArray[1] = (uint8_t)(number % 10);
		numberArray[0] = (uint8_t)(number/10);
	}
	else if( number < 1000)
	{
		numOfDigits = 3;
		numberArray[2] = (uint8_t)(number % 10);
		numberArray[1] = (uint8_t)((number/10)%10);
		numberArray[0] = (uint8_t)(number/100);		
	}
	else if( number < 10000)
	{
		numOfDigits = 4;
		numberArray[3] = (uint8_t)(number % 10);
		numberArray[2] = (uint8_t)((number/10)%10);
		numberArray[1] = (uint8_t)((number/100)%10);
		numberArray[0] = (uint8_t)(number/1000);		
	}
	
	
	
	for(int i = 0; i < numOfDigits; i++)
	{
		numberArray[i] = numberArray[i] | 0x30;
		
		pin_write(RS.port, RS.pin, HIGH);
		pin_write(RW.port, RW.pin, LOW);
	
		pin_write(D4.port, D4.pin, (numberArray[i]>>4)&0x01);
		pin_write(D5.port, D5.pin, (numberArray[i]>>5)&0x01);
		pin_write(D6.port, D6.pin, (numberArray[i]>>6)&0x01);
		pin_write(D7.port, D7.pin, (numberArray[i]>>7)&0x01);
	
		lcd_clock();
		//low bits
		pin_write(RS.port, RS.pin, HIGH);
		pin_write(RW.port, RW.pin, LOW);
	
		pin_write(D4.port, D4.pin, (numberArray[i])&0x01);
		pin_write(D5.port, D5.pin, (numberArray[i]>>1)&0x01);
		pin_write(D6.port, D6.pin, (numberArray[i]>>2)&0x01);
		pin_write(D7.port, D7.pin, (numberArray[i]>>3)&0x01);
		lcd_clock();	
	}

}
	
void lcd_clear(uint8_t row, uint8_t start_collumn, uint8_t end_collumn)
{
	lcd_set_cursor(row, start_collumn);
	
	for(int i = 0; i < (end_collumn-start_collumn)+1; i++)
	{
		lcd_write_char(' ');
	}
	
}

void lcd_set_cursor(uint8_t yPos, uint8_t xPos)
{
	if(yPos == 1)
	{
		xPos += 0x40; 
	}
	
	lcd_command(0x80 | xPos);
	
}

void lcd_create_character(uint8_t address, uint8_t character[])
{
	address = 8*address; //the CG address is 6 bits, the first 3 bits represent the specific 
											 //address of the custom character, and the last 3  bits represent
											 //the address of the graphical bit values
	
	lcd_command(0x40 | address); //the 0x40 is the write to CGRAM command, followed by the address
	
	for(int i = 0; i < 8; i++)
	{
		lcd_write_char(character[i]);
	}
}

