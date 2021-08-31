#ifndef GPIO_Driver
#define	GPIO_Driver

#include "stm32f10x.h"
#include <stdint.h>

//STATES
#define LOW 0
#define HIGH 1

//PORT NAMES
#define PORTA			GPIOA
#define PORTB			GPIOB
#define PORTC			GPIOC
#define PORTD			GPIOD
#define PORTE			GPIOE
#define PORTF			GPIOF
#define PORTG			GPIOG

//PIN MODES
#define OUTPUT       						((int32_t)0x01)
#define INPUT 									((uint32_t)0x00)

//INPUT TYPES
#define INPUT_ANALOG						((uint32_t)0x00)
#define INPUT_FLOATING					((uint32_t)0x01)
#define INPUT_PU_PD 						((uint32_t)0x02)

//OUTPUT TYPES
#define OUTPUT_GEN_PURPOSE 			((uint32_t)0x00)
#define OUTPUT_OD 					  	((uint32_t)0x01)
#define OUTPUT_ALT_FUNCTION 		((uint32_t)0x02)
#define OUTPUT_ALT_FUNCTION_OD	((uint32_t)0x03)

//PIN speed/slew rates
#define SPEED_2MHZ 						((uint32_t)0x02)
#define SPEED_10MHZ				 		((uint32_t)0x01)
#define SPEED_50MHZ 					((uint32_t)0x03)

//CLOCK enables
#define CLOCK_GPIO_EN_ALT_FUNC		(RCC->APB2ENR |= (1<<0))
#define CLOCK_GPIO_EN_PORTA   		(RCC->APB2ENR |= (1<<2))
#define CLOCK_GPIO_EN_PORTB   		(RCC->APB2ENR |= (1<<3))
#define CLOCK_GPIO_EN_PORTC   		(RCC->APB2ENR |= (1<<4))
#define CLOCK_GPIO_EN_PORTD   		(RCC->APB2ENR |= (1<<5))

//CONFIG REG CNF AND MODE
#define MODE_POS_BIT1 		(PINPOS[pinNumber])
#define MODE_POS_BIT2			(PINPOS[pinNumber] + 1)
#define CNF_POS_BIT1			(PINPOS[pinNumber] + 2)
#define CNF_POS_BIT2			(PINPOS[pinNumber] + 3)

//CONFIG STRUCT
typedef struct 
{
	GPIO_TypeDef *port;
	uint32_t pin;
	uint32_t mode;
	uint32_t mode_type;
	uint32_t pull; 
	uint32_t speed;
	uint32_t alt;
}	GPIO_TYPE;

//EDGE ENUM
typedef enum
{
	RISING_EDGE,
	FALLING_EDGE,
	RISING_FALLING_EDGE,
}edge_select;

//FUNCTION PROTOTYPES

//====================================
//       GPIO CONFIGURATIONS
//====================================

void config_pin(GPIO_TypeDef *port, uint32_t pinNumber, uint32_t modeType, uint32_t pinSpeed, uint32_t mode);
void init_gpio(GPIO_TYPE gpio_type);

//=====================================
//         GPIO COMMANDS
//=====================================

void pin_write(GPIO_TypeDef *port, uint32_t pinNumber, uint32_t state);
void pin_toggle(GPIO_TypeDef *port, uint32_t pinNumber);
uint32_t pin_read(GPIO_TypeDef *port, uint32_t pinNumber);

//=====================================
//      GPIO INTERRUPT COMMANDS
//=====================================

void config_gpio_interrupt(GPIO_TypeDef *port, uint32_t pinNumber, edge_select edge);
void enable_gpio_interrupt(uint32_t pinNumber, IRQn_Type irqNumber);
void clear_gpio_interrupt(uint32_t pinNumber);

//=====================================
//        SLEEP FUNCTIONS
//=====================================

void goToSleep(void);


#endif
