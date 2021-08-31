//THIS LIBRARY USES THE I2C BUS LOCATED ON PINS B10 AND B11

#ifndef I2C_Driver
#define	I2C_Driver

#include "stm32f10x.h"
#include <stdint.h>


//the i2c_init function writes the required bits to set up the I2C mode 2 with
//a 100Khz frequency and enables clock stretching
void i2c_init(void);


//i2c_write takes in the device address, the memory address of said device,
//and the data that the user wants to write to said memory address. 
void i2c_write(uint8_t device_address, uint8_t mem_address, uint8_t data);

//i2c_write_2 writes to devices that have a high and low address width, such as EEPROM. 
void i2c_write_2(uint8_t device_address, uint8_t mem_address_hi, uint8_t mem_address_lo, uint8_t data);

//i2c_read takes in the device address and the memory address of said device and
//returns the data that is stored in said memory address 
uint8_t i2c_read(uint8_t device_address, uint8_t mem_address);


#endif

