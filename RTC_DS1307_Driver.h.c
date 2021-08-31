#ifndef RTC_Driver
#define RTC_Driver

#include "stm32f10x.h"
#include "system_stm32f10x.h"

//Macros to define the registers in the DS1307 module
#define RTC_ADDRESS 0xD0	
#define SECOND_REG 	0x00
#define MINUTE_REG 	0x01
#define HOUR_REG 		0x02
#define DAY_REG			0x03
#define DATE_REG 		0x04
#define MONTH_REG		0x05
#define YEAR_REG		0x06

//The rtc_init function simply calls the i2c_init function in the I2C_Driver
void rtc_init(void);

//the rtc_set_time function takes in the hours, minutes, and seconds and
//writes this data to the correct locations in the DS1307 in order to
//modify the time of day
void rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);

//the rtc_set_date function takes in the month, date, and year and writes
//this data to the correct locations in the DS1307 in order to modify the
//month, date, and year
void rtc_set_date(uint8_t month, uint8_t date, uint8_t year);

//the rtc_get_seconds function returns the value in the SECOND_REG
uint8_t rtc_get_seconds(void);

//the rtc_get_minutes function returns the value in the MINUTE_REG
uint8_t rtc_get_minutes(void);

//the rtc_get_hours function returns the value in the HOUR_REG
uint8_t rtc_get_hours(void);

//the rtc_get_date function returns the value in the DATE_REG
uint8_t rtc_get_date(void);

//the rtc_get_month function returns the value in the MONTH_REG
uint8_t rtc_get_month(void);

//the rtc_get_year function returns the value in the YEAR_REG
uint8_t rtc_get_year(void);




#endif

