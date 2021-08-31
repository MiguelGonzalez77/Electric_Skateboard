#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "I2C_Driver.h"
#include "RTC_Driver.h"

void rtc_init(void)
{
	i2c_init();
}

void rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	uint8_t seconds_HEX = 0;
	uint8_t minutes_HEX = 0;
	uint8_t hours_HEX = 0;
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers in decimal
	//to hex before being sent.
	seconds_HEX = ((seconds/10) * 16) + (seconds%10);
	minutes_HEX = ((minutes/10) * 16) + (minutes%10);
	hours_HEX =   ((hours/10) * 16) + (hours%10);	
	
	i2c_write(RTC_ADDRESS, SECOND_REG, seconds_HEX);
	i2c_write(RTC_ADDRESS, MINUTE_REG, minutes_HEX);
	i2c_write(RTC_ADDRESS, HOUR_REG,	hours_HEX);
}	

void rtc_set_date(uint8_t month, uint8_t date, uint8_t year)
{
	uint8_t month_HEX = 0;
	uint8_t date_HEX = 0;
	uint8_t year_HEX = 0;
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers in decimal
	//to hex before being sent.
	month_HEX = ((month/10) * 16) + (month%10);
	date_HEX = ((date/10) * 16) + (date%10);
	year_HEX =   ((year/10) * 16) + (year%10);	
	
	i2c_write(RTC_ADDRESS, MONTH_REG, month_HEX);
	i2c_write(RTC_ADDRESS, DATE_REG, date_HEX);
	i2c_write(RTC_ADDRESS, YEAR_REG, year_HEX);
}	

uint8_t rtc_get_seconds(void)
{
	uint8_t seconds;
	seconds = i2c_read(RTC_ADDRESS, SECOND_REG);
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers from hex
	//to decimal before being returned.
	seconds = ((seconds >> 4) * 10) + (seconds & 0x0F);
	
	return seconds;
}

uint8_t rtc_get_minutes(void)
{
	uint8_t minutes;
	minutes = i2c_read(RTC_ADDRESS, MINUTE_REG);
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers from hex
	//to decimal before being returned.
	minutes = ((minutes >> 4) * 10) + (minutes & 0x0F);
	
	return minutes;
}

uint8_t rtc_get_hours(void)
{
	uint8_t hours;
	hours = i2c_read(RTC_ADDRESS, HOUR_REG);
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers from hex
	//to decimal before being returned.
	hours = ((hours >> 4) * 10) + (hours & 0x0F);
	
	return hours;
}

uint8_t rtc_get_date(void)
{
	uint8_t date;
	date = i2c_read(RTC_ADDRESS, DATE_REG);
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers from hex
	//to decimal before being returned.
	date = ((date >> 4) * 10) + (date & 0x0F);
	
	return date;
}

uint8_t rtc_get_month(void)
{
	uint8_t month;
	month = i2c_read(RTC_ADDRESS, MONTH_REG);
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers from hex
	//to decimal before being returned.
	month = ((month >> 4) * 10) + (month & 0x0F);
	
	return month;
}

uint8_t rtc_get_year(void)
{
	uint8_t year;
	year = i2c_read(RTC_ADDRESS, YEAR_REG);
	
	//the DS1307 module stores hexadceimal values in its 
	//registers; for example, 30 seconds in the
	//DS1307 module is 0x30. However in decimal, this is 
	//48. Therefor the code below changes the numbers from hex
	//to decimal before being returned.
	year = ((year >> 4) * 10) + (year & 0x0F);
	
	return year;
}

