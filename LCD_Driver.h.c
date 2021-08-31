/Here is an example of a custom character declaration 
//	uint8_t static battery[8] =
//	{
//		0x04,	//0b00000100,
//		0x1F,	//0b00011111,
//		0x11,	//0b00010001,
//		0x11,	//0b00010001,
//		0x1F,	//0b00011111,
//		0x1F,	//0b00011111,
//		0x1F,	//0b00011111,
//		0x1F,	//0b00011111
//	};

#ifndef LCD_Driver
#define	LCD_Driver

#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "GPIO_Driver.h"

#define EIGHT_BIT_MODE 	0x30
#define FOUR_BIT_MODE 	0x28
#define DISPLAY_OFF			0x08
#define DISPLAY_CLEAR 	0x01
#define EM_INCREMENT		0x06
#define EM_DECREMENT 		0x04
#define DISPLAY_ON_CON 	0x0F
#define DISPLAY_ON_COFF	0x0C
#define SHIFT_RIGHT			0x14
#define SHIFT_LEFT			0x10

//The lcd_init function initizlizees the LCD to the following modes:
// - four pin operation
// - increment mode 
// - display on with the cursor off
//The user passes in the desired ports and pins that correlate to the different
//pins on the LCD screen. The lcd_init function will handle the gpio initilzation as well
void lcd_init(GPIO_TypeDef *D4port, uint8_t D4pin, GPIO_TypeDef *D5port, uint8_t D5pin, 
							GPIO_TypeDef *D6port, uint8_t D6pin, GPIO_TypeDef *D7port, uint8_t D7pin,
							GPIO_TypeDef *ENport, uint8_t ENpin, GPIO_TypeDef *RWport, uint8_t RWpin,
							GPIO_TypeDef *RSport, uint8_t RSpin);

//The lcd_clock function writes a logical 1 to the EN pin for 1 ms and then writes a logical 0 to
//the EN pin and waits for 1 ms. This satisfies the timing requiremnts in the datasheet for 
//my LCD module (HD44780U). This should be called whenever data wants to be sent to the LCD module
//NOTE: all the functions include this command so the user rarely ever calls this function
void lcd_clock(void);

//The lcd_command function takes in a byte that corresponds to a specific LCD command. This
//function is different from the lcd_write_char function based on the fact that in the datasheet,
//it says that a command is written when the RS pin is low, where as a character is written when the
//RS pin is high
void lcd_command(uint8_t command);

//The lcd_write_char function takes in a byte that corresponds to a character in the 
//LCDs DDRAM or CGRAM and prints it on the screen 
void lcd_write_char(uint8_t character);

//The write number function takes a number with the size of 2 bytes and prints out the correct
//number onto the screen. This function handles the converstion from a regular variable to 
//appropriate characters in the LCDs DDRAM
void lcd_write_number(uint16_t number);

//The lcd_set_cursor function sets the position of the lcd cursor to the user specified position
//(row first, then collumn)
void lcd_set_cursor(uint8_t yPos, uint8_t xPos);

//The lcd_clear function writes an empty space character to the user specfied position. For
//example, if the user wants to clear the entire first row, the user would say
//lcd_clear(0, 0, 16)
void lcd_clear(uint8_t row, uint8_t start_collumn, uint8_t end_collumn);

//The lcd_create_character function takes in an address (0-7) and a "character" pointer that 
//is defined by the user (an example of this can be found at the top of the header file. 
//The function then loads the character inside the LCDs CGRAM and can be displayed by calling 
//the lcd_write_char function with the input being the address of the character (0-7)
void lcd_create_character(uint8_t address, uint8_t *character);


#endif


