#include "GPIO_Driver.h"
#include <stdint.h>

uint32_t static PINPOS[16] = 
{										 
	(0x00), 
  (0x04),
	(0x08),
	(0x0C),
	(0x10),
	(0x14),
	(0x18),
	(0x1C),
	(0x00), 
	(0x04),
	(0x08),
	(0x0C),
	(0x10),
	(0x14),
	(0x18),
	(0x1C)
};

//=====================================================================================================================
//													GPIO CONFIGURATIONS
//=====================================================================================================================
void config_pin(GPIO_TypeDef *port, uint32_t pinNumber, uint32_t mode, uint32_t pinSpeed, uint32_t modeType)
{
	if(pinNumber >= 8) 
	{
			
		switch(modeType)
		{
				
			//OUTPUT and INPUT MODES
			case OUTPUT_GEN_PURPOSE | INPUT_ANALOG:
				port->CRH &= ~( (1<<CNF_POS_BIT1) | (1<<CNF_POS_BIT2) );
			break;
			
			case OUTPUT_OD | INPUT_FLOATING:
				port->CRH &= ~(1<<CNF_POS_BIT2);
				port->CRH |= (1<<CNF_POS_BIT1); 
			break;
			
			case OUTPUT_ALT_FUNCTION | INPUT_PU_PD:
				port->CRH |= (1<<CNF_POS_BIT2);  
				port->CRH &= ~(1<<CNF_POS_BIT1);
			break;
			
			case OUTPUT_ALT_FUNCTION_OD:
				port->CRH |= (1<<CNF_POS_BIT2);  
				port->CRH |= (1<<CNF_POS_BIT1);
			break;
		}
		
		if(mode == INPUT)
		{
			port->CRH &= ~( (1<<MODE_POS_BIT1) | (1<<MODE_POS_BIT2) );
		}
		else
		{
			port->CRH |= ( pinSpeed << MODE_POS_BIT1 );
		}
		
	}
	else 
	{
		
		switch(modeType)
		{
			
			//OUTPUT and INPUT MODES
			case OUTPUT_GEN_PURPOSE | INPUT_ANALOG:
				port->CRL &= ~( (1<<CNF_POS_BIT1) | (1<<CNF_POS_BIT2) );
			break;
			
			case OUTPUT_OD | INPUT_FLOATING:
				port->CRL &= ~(1<<CNF_POS_BIT2);
				port->CRL |= (1<<CNF_POS_BIT1);
			break;
				
			case OUTPUT_ALT_FUNCTION | INPUT_PU_PD:
				port->CRL |= (1<<CNF_POS_BIT2);  
				port->CRL &= ~(1<<CNF_POS_BIT1);
			break;
			
			case OUTPUT_ALT_FUNCTION_OD:
				port->CRL |= (1<<CNF_POS_BIT2);  
				port->CRL |= (1<<CNF_POS_BIT1);
			break;
		}
		
		if(mode == INPUT)
		{
			port->CRL &= ~( (1<<MODE_POS_BIT1) | (1<<MODE_POS_BIT2) );
		}
		else
		{
			port->CRL |= ( pinSpeed << MODE_POS_BIT1 );
		}
		
	}
	
}

void init_gpio(GPIO_TYPE gpio_type)
{
	
	if(gpio_type.port == PORTA)
	{
		CLOCK_GPIO_EN_PORTA;
	}
	else if(gpio_type.port == PORTB)
	{
		CLOCK_GPIO_EN_PORTB;
	}
	else if(gpio_type.port == PORTC)
	{
		CLOCK_GPIO_EN_PORTC;
	}
	else if(gpio_type.port == PORTD)
	{
		CLOCK_GPIO_EN_PORTD;
	}
	
	config_pin(gpio_type.port, gpio_type.pin, gpio_type.mode, gpio_type.speed, gpio_type.mode_type);
	
}

//=====================================================================================================================
//         											GPIO COMMANDS
//=====================================================================================================================
void pin_write(GPIO_TypeDef *port, uint32_t pinNumber, uint32_t state)
{
	
	if(state == 1)
	{
		port->BSRR = (1<<pinNumber);
	}
	else
	{
		port->BSRR = (1<<(pinNumber+16));
	}
	
}

void pin_toggle(GPIO_TypeDef *port, uint32_t pinNumber)
{
	
	port->ODR ^= (1<<pinNumber);
	
}

uint32_t pin_read(GPIO_TypeDef *port, uint32_t pinNumber)
{
	uint32_t bitStatus;
	
	if((port->IDR & (1<<pinNumber)) != (uint32_t)LOW)
  {
		bitStatus = 1;
  }
  else
  {
    bitStatus = 0;
  }
  return bitStatus;
}

//=====================================================================================================================
//														GPIO INTERRUPT COMMANDS
//=====================================================================================================================
void config_gpio_interrupt(GPIO_TypeDef *port, uint32_t pinNumber, edge_select edge)
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; //set timer for the AFIO 
	
	//set configurations for port A
	if(port == PORTA)
	{
		switch(pinNumber)
		{
			
			case 0:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI0_PA;
			break;
			
			case 1:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI1_PA;
			break;
			
			case 2:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI2_PA;
			break;
			
			case 3:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI3_PA;
			break;
			
			case 4:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PA;
			break;
			
			case 5:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI5_PA;
			break;
			
			case 6:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI6_PA;
			break;
			
			case 7:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI7_PA;
			break;
			
			case 8:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI8_PA;
			break;
			
			case 9:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI9_PA;
			break;
				
			case 10:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI10_PA;
			break;
			
			case 11:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI11_PA;
			break;
			
			case 12:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PA;
			break;
			
			case 13:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI13_PA;
			break;
			
			case 14:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI14_PA;
			break;
			
			case 15:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI15_PA;
			break;
		}
	}
	
	//set configurations for port B
	if(port == PORTB)
	{
		switch(pinNumber)
		{
			
			case 0:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI0_PB;
			break;
			
			case 1:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI1_PB;
			break;
			
			case 2:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI2_PB;
			break;
			
			case 3:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI3_PB;
			break;
			
			case 4:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PB;
			break;
			
			case 5:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI5_PB;
			break;
			
			case 6:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI6_PB;
			break;
			
			case 7:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI7_PB;
			break;
			
			case 8:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI8_PB;
			break;
			
			case 9:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI9_PB;
			break;
				
			case 10:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI10_PB;
			break;
			
			case 11:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI11_PB;
			break;
			
			case 12:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PB;
			break;
			
			case 13:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI13_PB;
			break;
			
			case 14:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI14_PB;
			break;
			
			case 15:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI15_PB;
			break;
		}
	}
	
	//set configurations for port C
	if(port == PORTC)
	{
		switch(pinNumber)
		{
			
			case 0:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI0_PC;
			break;
			
			case 1:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI1_PC;
			break;
			
			case 2:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI2_PC;
			break;
			
			case 3:
				AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI3_PC;
			break;
			
			case 4:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PC;
			break;
			
			case 5:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI5_PC;
			break;
			
			case 6:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI6_PC;
			break;
			
			case 7:
				AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI7_PC;
			break;
			
			case 8:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI8_PC;
			break;
			
			case 9:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI9_PC;
			break;
				
			case 10:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI10_PC;
			break;
			
			case 11:
				AFIO->EXTICR[2] = AFIO_EXTICR3_EXTI11_PC;
			break;
			
			case 12:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PC;
			break;
			
			case 13:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI13_PC;
			break;
			
			case 14:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI14_PC;
			break;
			
			case 15:
				AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI15_PC;
			break;
		}
	}
	
	//configure the edge of the interrupt
	if(edge == RISING_EDGE)
	{
		EXTI->RTSR |= (1<<pinNumber);
	}
	else if(edge == FALLING_EDGE)
	{
		EXTI->FTSR |= (1<<pinNumber);
	}
	else if(edge == RISING_FALLING_EDGE)
	{
		EXTI->RTSR |= (1<<pinNumber);
		EXTI->FTSR |= (1<<pinNumber);
	}
	
}

void enable_gpio_interrupt(uint32_t pinNumber, IRQn_Type irqNumber)
{
	//enable interrupt in the EXTI
	EXTI->IMR |= (1<<pinNumber);
	
	//enable interupt in the NVIC
	NVIC_EnableIRQ(irqNumber);
}



void clear_gpio_interrupt(uint32_t pinNumber)
{
	EXTI->PR |= (1<<pinNumber);
}
//=====================================================================================================================
// 									               	SLEEP FUNCTIONS
//=====================================================================================================================

void goToSleep(void)
{
	  //enable control clock for PWR
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	
		//set deep sleep bit in the Cortex System Control register
		SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	
		//set Standby mode
		PWR->CR |= PWR_CR_PDDS;
	
		//clear wake up flag
		PWR->CR |= PWR_CR_CWUF;
	
		//enable wake up pin
		PWR->CSR |= PWR_CSR_EWUP;

		//put the STM to sleep
		__WFI();
	

}

