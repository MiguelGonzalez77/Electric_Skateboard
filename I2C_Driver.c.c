#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "I2C_Driver.h"

void i2c_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	
	//configure the correct GPIO setting for the SDA and SCL pins of I2C mode 2
	GPIOB->CRH |= GPIO_CRH_CNF10 | GPIO_CRH_MODE10 | GPIO_CRH_CNF11 | GPIO_CRH_MODE11;
	GPIOB->ODR |= (1<<10) | (1<<11); 
	
	RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; //enables clock for the I2C 2 bus
	
	I2C2->CR2 |= I2C_CR2_FREQ_5 | I2C_CR2_FREQ_2; //sets i2c freq to 36Mhz (because the APB1 clock runs at 36Mhz)
	
	I2C2->CCR |= 180; //(CCR * Tpclock) = (Tpi2c/2) -->>  (180 * (1/36Mhz)) = (1/100kHz)(0.5)
	I2C2->TRISE = 37; //(1000ns / Tpclock) + 1 = TRISE -->> (1000ns/27.7ns) + 1 = 37 
	
	I2C2->CR1 &= ~(I2C_CR1_NOSTRETCH);//enables clock stretching 	
	
	I2C2->CR1 |= I2C_CR1_ACK | I2C_CR1_PE; //starts the I2C bus and enables ACKs
	
}

void i2c_write(uint8_t device_address, uint8_t mem_address, uint8_t data)
{
	uint16_t temp;
	
	I2C2->CR1 |= I2C_CR1_START; //generate start condition
	while(!(I2C2->SR1 & I2C_SR1_SB)) //wait for the i2c to send the start condition
	{
	}
	temp = I2C2->SR1;
	I2C2->DR = device_address | 0x00; //send the adress plus a write opperation (0x01)

	while(!(I2C2->SR1 & I2C_SR1_ADDR)) //wait for the i2c to get a match for the address
	{
	}
	temp = I2C2->SR1;
	temp = I2C2->SR2;
	
	I2C2->DR = mem_address;
	while(!(I2C2->SR1 & I2C_SR1_BTF)) //wait for the i2c to send the mem_address
	{
	}
	
	I2C2->CR1 &= ~(I2C_CR1_ACK); //This sends a NACK to the slave, which tells it that this is the last read
	I2C2->DR = data;
	while(!(I2C2->SR1 & I2C_SR1_BTF)) //wait for the i2c to send the data
	{
	}
	
	I2C2->CR1 |= I2C_CR1_STOP; //generates a stop condition
	
}

void i2c_write_2(uint8_t device_address, uint8_t mem_address_hi, uint8_t mem_address_lo, uint8_t data)
{
	uint16_t temp;
	
	I2C2->CR1 |= I2C_CR1_START; //generate start condition
	while(!(I2C2->SR1 & I2C_SR1_SB)) //wait for the i2c to send the start condition
	{
	}
	temp = I2C2->SR1;
	I2C2->DR = device_address | 0x00; //send the adress plus a write opperation (0x00)

	while(!(I2C2->SR1 & I2C_SR1_ADDR)) //wait for the i2c to get a match for the address
	{
	}
	temp = I2C2->SR1;
	temp = I2C2->SR2;
	
	I2C2->DR = mem_address_hi;
	while(!(I2C2->SR1 & I2C_SR1_BTF)) //wait for the i2c to send the mem_address
	{
	}
	
	I2C2->DR = mem_address_lo;
	while(!(I2C2->SR1 & I2C_SR1_BTF)) //wait for the i2c to send the mem_address
	{
	}
	
	I2C2->CR1 &= ~(I2C_CR1_ACK); //This sends a NACK to the slave, which tells it that this is the last read
	I2C2->DR = data;
	
	while(!(I2C2->SR1 & I2C_SR1_BTF)) //wait for the i2c to send the data
	{
	}
	
	I2C2->CR1 |= I2C_CR1_STOP; //generates a stop condition
}

uint8_t i2c_read(uint8_t device_address, uint8_t mem_address)
{
	uint16_t temp;
	
	I2C2->CR1 |= I2C_CR1_START; //generate start condition
	while(!(I2C2->SR1 & I2C_SR1_SB)) //wait for the i2c to send the start condition
	{
	}
	temp = I2C2->SR1; 
	I2C2->DR = device_address | 0x00; //send the adress plus a write opperation (0x00)

	while(!(I2C2->SR1 & I2C_SR1_ADDR)) //wait for the i2c to get a match for the address
	{
	}
	temp = I2C2->SR1;
	temp = I2C2->SR2; //reading from SR2 clears the ADDR flag
	
	I2C2->DR = mem_address;
	while(!(I2C2->SR1 & I2C_SR1_BTF)) //wait for the i2c to send the mem_address
	{
	}
	
	I2C2->CR1 |= I2C_CR1_START; //generate repeat start condition
	
	while(!(I2C2->SR1 & I2C_SR1_SB)) //wait for the i2c to send the repeat start condition
	{
	}
	temp = I2C2->SR1;
	I2C2->DR = device_address | 0x01; //send the adress plus a read opperation (0x01)

	while(!(I2C2->SR1 & I2C_SR1_ADDR)) //wait for the i2c to get a match for the address
	{
	}
	I2C2->CR1 &= ~(I2C_CR1_ACK); //This sends a NACK to the slave, which tells it that this is the last read
	temp = I2C2->SR1;
	temp = I2C2->SR2; //reading from SR2 clears the ADDR flag
		
	while((I2C2->SR1 & I2C_SR1_ADDR)) //wait for the addr bit to be cleared
	{
	}
	I2C2->CR1 |= I2C_CR1_STOP; //generates a stop condition
	
	while((I2C2->SR2 & I2C_SR2_BUSY)) //wait until the bus is not busy 
	{
	}
	
	temp = (uint8_t)I2C2->DR;
	return (uint8_t)temp;
}


