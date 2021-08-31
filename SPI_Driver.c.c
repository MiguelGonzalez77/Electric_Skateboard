#include "stm32f10x.h" 
#include "GPIO_Driver.h" 
#include "SPI_drive.h"
#include "delay.h"

void spi_write(char data)
{		
		uint8_t dumby_read = 0;
	
		dumby_read = (uint8_t) SPI1->DR; //this is neccessary to prevent OVR errors
	
		SPI1->DR = data;

		while( SPI1->SR != 0x0002 )
		{
			dumby_read = (uint8_t) SPI1->DR;
		}	
		
}


uint8_t spi_read()
{
	return (uint8_t) SPI1->DR;
}

void spi_init_master()
{
	//enable clocks for SPI1, PORTA, and Alternate Function IO
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
	
	//configure the MOSI pin
	GPIO_TYPE MOSI;
	MOSI.port = PORTA;
	MOSI.pin = 7;
	MOSI.mode = OUTPUT;
	MOSI.mode_type = OUTPUT_ALT_FUNCTION;
	MOSI.speed = SPEED_10MHZ;
	
	//configure the MISO pin
	GPIO_TYPE MISO;
	MISO.port = PORTA;
	MISO.pin = 6;
	MISO.mode = INPUT;
	MISO.mode_type = INPUT_PU_PD;
	
	//configure the SCK pin
	GPIO_TYPE SCK;
	SCK.port = PORTA;
	SCK.pin = 5;
	SCK.mode = OUTPUT;
	SCK.mode_type = OUTPUT_ALT_FUNCTION;
	SCK.speed = SPEED_10MHZ;
	
	//initialize all of the pins
	
	init_gpio(MOSI);
	init_gpio(MISO);
	init_gpio(SCK);
	
	//confiure SPI1 for master
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0 | SPI_CR1_MSTR;
	SPI1->CR2 |= SPI_CR2_SSOE;
	
	//enable SPI
	SPI1->CR1 |= SPI_CR1_SPE;

}
