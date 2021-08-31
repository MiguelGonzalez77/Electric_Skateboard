#include "stm32f10x.h"

/*
Pin setup for SPI mode 1

PA4 --> SS
PA5 --> SCLK
PA6 --> MISO
PA7 --> MOSI
*/

//=====================================================================================================
//																		FUNCTION PROTOTYPES
//=====================================================================================================

//*****************************************************************************************************
void spi_init_master(void);
																	
//		The spi_init_master function enables SPI1 to the lowest speed setting (prescaler of 256), 
//		enables master mode, enables full duplex, and enables software slave managment. To change
//		any of these settings, write the specific bytes into the SPI1->CR1 register.
//*****************************************************************************************************



//*****************************************************************************************************
void spi_write(char data);

//		The spi_write function sends 1 byte of data over the SPI line
//*****************************************************************************************************



//*****************************************************************************************************
uint8_t spi_read(void);

//		Because SPI is bidirectional, The spi_read function just returns whatever is in the SPI's 
//		data register.
//*****************************************************************************************************

