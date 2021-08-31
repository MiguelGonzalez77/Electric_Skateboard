#ifndef EEPROM_Driver
#define	EEPROM_Driver

#include "stm32f10x.h"
#include <stdint.h>

//The eeprom_read function takes in the EEPROM's device address and the hi
//and low bytes of the internal address and returns the value of what is 
//inside said internal address
uint8_t eeprom_read(uint8_t device_address, uint8_t addr_hi, uint8_t addr_lo);

//The eeprom_write function takes in the EEPROM's device address, the hi
//and low bytes the internal address, and the data that the user wants
//to store in said internal address
void eeprom_write(uint8_t device_address, uint8_t addr_hi, uint8_t addr_lo, uint8_t data);

//eeprom_init simply calls the i2c_init function in the I2C_Driver. 
void eeprom_init(void);

#endif
