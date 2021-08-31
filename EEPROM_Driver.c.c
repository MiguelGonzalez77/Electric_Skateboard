#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "EEPROM_Driver.h"
#include "I2C_Driver.h"
#include "delay.h"


//The reason I have to do extra stuff with the I2C library is because of the fact that
//the I2C on the STM32 requires different code for different loads of data; For example,
//sending one byte over the I2C line requires different code for sending 2 bytes of data,
//and the same goes for 3 bytes of data. I wonder why there is not a repeat start
//condition after one byte of data has been sent. If there was, then I could just keep calling 
//repeat start conditions until I was satisfied with the data that has been sent, and then
//I could just send a nack or a stop condition.
uint8_t eeprom_read(uint8_t device_address, uint8_t addr_hi, uint8_t addr_lo)
{
	i2c_write(device_address, addr_hi, addr_lo);
	
	uint8_t data;
	uint16_t temp;
	
	I2C2->CR1 |= I2C_CR1_START; //generate start condition
	while(!(I2C2->SR1 & I2C_SR1_SB)) //wait for the i2c to send the start condition
	{
	}
	temp = I2C2->SR1;
	I2C2->DR = device_address | 0x01; //send the adress plus a read opperation (0x01)

	while(!(I2C2->SR1 & I2C_SR1_ADDR)) //wait for the i2c to get a match for the address
	{
	}
	I2C2->CR1 &= ~(I2C_CR1_ACK); //This sends a NACK to the slave, which tells it that this is the last read
	temp = I2C2->SR1;
	temp = I2C2->SR2;
	I2C2->CR1 |= I2C_CR1_STOP; //generates a stop condition
	
	while(!(I2C2->SR1 & I2C_SR1_RXNE)) //wait for the i2c to get the data from the EEPROM
	{
	}

	data = (uint8_t)I2C2->DR;
	
	return data;

}

void eeprom_write(uint8_t device_address, uint8_t addr_hi, uint8_t addr_lo, uint8_t data)
{
	i2c_write_2(device_address, addr_hi, addr_lo, data);
	delayMs(6); //data sheet says that eeprom write operation takes 5 ms. I put 6 just to be safe
}

void eeprom_init()
{
	i2c_init();
}
	
