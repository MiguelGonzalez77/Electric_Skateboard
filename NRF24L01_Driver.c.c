#include "stm32f10x.h"
#include "GPIO_Driver.h"
#include "SPI_drive.h"
#include "NRF_Driver.h"
#include "delay.h"

const int static CSN_DELAY_TIME = 25; //This is the delay time, in micro seconds, that is required before 
                               //bringing the CSN pin high. I don't really know why I have to delay
															 //for such a short amount of time right before the CSN pin is pulled high.
															 //I know it's not an spi timing issue because an spi read/write
															 //can be called multiple times before writing high to the CSN pin. 
															 //I also tried reducing the speed of the CSN pin down to 2Mhz; however, 
															 //this change had no effect.

GPIO_TYPE static CE;
GPIO_TYPE  static CSN;

void nrf_init_pins(GPIO_TypeDef *CE_port, uint32_t CE_pin, GPIO_TypeDef *CSN_port, uint32_t CSN_pin)
{
	//configure the CE pin
	CE.port = CE_port;
	CE.pin = CE_pin;
	CE.mode = OUTPUT;
	CE.mode_type = OUTPUT_GEN_PURPOSE;
	CE.speed = SPEED_50MHZ;	
	
	//configure the CSN pin
	CSN.port = CSN_port;
	CSN.pin = CSN_pin;
	CSN.mode = OUTPUT;
	CSN.mode_type = OUTPUT_GEN_PURPOSE;
	CSN.speed = SPEED_50MHZ;	

	init_gpio(CE);
	init_gpio(CSN);
	
	pin_write(CE.port, CE.pin, LOW); //bring CE Low

	pin_write(CSN.port, CSN.pin, HIGH);  //bring CSN HIGH
	
	spi_init_master();
	
}	

void nrf_write_reg(char reg, char data)
{
		pin_write(CSN.port, CSN.pin, LOW);
		
		spi_write(W_REGISTER | reg); 
		
		spi_write(data); 
	
		delayUs(CSN_DELAY_TIME);
		pin_write(CSN.port, CSN.pin, HIGH);
	
}

uint8_t nrf_read_reg(char reg)
{
		uint8_t reg_data = 0;
	
		pin_write(CSN.port, CSN.pin, LOW);
		
		spi_write(R_REGISTER | reg); 
		
		spi_write(0xF1); //send dumby byte in order to recieve the data inside the reg
		
		delayUs(CSN_DELAY_TIME);
		pin_write(CSN.port, CSN.pin, HIGH); 
		
		reg_data = spi_read();
	
		return reg_data;
}

void nrf_init_transmiter(uint32_t addr_high, uint8_t addr_low, uint8_t channel, char en_auot_ack, char en_dpl)
{
	nrf_cmd_clear_interrupts();
	
	nrf_write_reg(NRF_CONFIG, 0x7A); 
	delayMs(100);

	nrf_write_reg(EN_AA, 0x3F);
	
	nrf_write_reg(EN_RXADDR, 0x03);
	
	nrf_write_reg(SETUP_AW, 0x03);
	
	nrf_write_reg(SETUP_RETR, 0xFF); 
	
	nrf_write_reg(RF_CH, channel);
	
	nrf_write_reg(RF_SETUP, 0x0F);
		
	nrf_set_tx_addr(addr_high, addr_low, en_auot_ack);
		
	if (en_dpl)
	{
		nrf_cmd_activate();
		nrf_write_reg_specific_bit(FEATURE, EN_DPL, 1);   
		nrf_write_reg_specific_bit(DYNPD, PIPE_0, 1);		
	}
}

void nrf_init_reciever(uint8_t pipe, uint32_t addr_high, uint8_t addr_low, uint8_t channel, char en_auto_ack, char en_dpl, uint8_t dpl_w)
{
	nrf_cmd_clear_interrupts();
	
	nrf_write_reg(NRF_CONFIG, 0x7B);
	delayMs(100);
	
	nrf_write_reg(EN_AA, 0x3F);

	nrf_write_reg(SETUP_AW, 0x03);
	
	nrf_write_reg(SETUP_RETR, 0xFF);
	
	nrf_write_reg(RF_CH, channel);
	
	nrf_write_reg(RF_SETUP, 0x0F);
		
	nrf_write_reg_specific_bit(EN_RXADDR, pipe, 1); //enable rx pipe 
	nrf_set_rx_addr(pipe, addr_high, addr_low);     //set address			
	
	if (en_dpl == 1) //if dynamic payload width is enabled
	{
		nrf_cmd_activate();
		nrf_write_reg_specific_bit(FEATURE, EN_DPL, 1); //enable dynamic PL feature
		nrf_write_reg_specific_bit(DYNPD, pipe, 1);     //enable dynamic PL for pipe
		
		//auto ack has to be enabled if using dynamic payload
		nrf_write_reg_specific_bit(EN_AA, pipe, 1); //enable auto ack on pipe 	
	}
	else
	{
		nrf_write_reg((pipe + RX_PW_OFFSET), dpl_w);   //write the static payload width
		
		//if using static PL width use can still use auto ack but now its optional
		if(en_auto_ack)
		{	
			nrf_write_reg_specific_bit(EN_AA, ENAA_P1, 1);  //enable auto ack on pipe 1	
			nrf_write_reg_specific_bit(EN_AA, pipe, 1);     //enable auto ack on pipe 
		}
	}
}

void nrf_write_reg_specific_bit(uint8_t reg, uint8_t bit, uint8_t value)
{	
	//READ
	uint8_t reg_value =  nrf_read_reg(reg);
	
	//MODIFY
	if (value == 1)
	{
		reg_value |= (1 << bit);
	}
	else
	{
		reg_value &= ~(1 << bit);
	}
		
	//WRITE
	nrf_write_reg(reg, reg_value);
}

void nrf_read_reg_multiple_bytes(uint8_t reg, uint8_t *data_buff)
{
	pin_write(CSN.port, CSN.pin, LOW);
	
	spi_write(reg); 
	
	spi_write(0xF1); 
	data_buff[0] = spi_read();
	
	spi_write(0xF1); 
	data_buff[1] = spi_read();
	
	spi_write(0xF1); 
	data_buff[2] = spi_read();
	
	spi_write(0xF1); 
	data_buff[3] = spi_read();
	
	spi_write(0xF1); 
	data_buff[4] = spi_read();
	
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH);
}

void nrf_cmd_listen(void)
{
	pin_write(CE.port, CE.pin, HIGH); //bring CE pin HIGH
}

uint8_t nrf_cmd_get_status(void)
{
	return nrf_read_reg(0x07);
}

void nrf_cmd_clear_interrupts(void)
{
	nrf_write_reg(0x07, 0x70);
}

void nrf_cmd_flush_tx(void)
{
	pin_write(CSN.port, CSN.pin, LOW);
		
	spi_write(0xE1); 
		
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH); 
}

void nrf_cmd_flush_rx(void)
{
	pin_write(CSN.port, CSN.pin, LOW);
		
	spi_write(0xE2); 
		
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH);
}

void nrf_cmd_act_as_RX(void)
{
	nrf_write_reg_specific_bit(0x00, 0, 1);
}

void nrf_cmd_act_as_TX(void)
{
	nrf_write_reg_specific_bit(0x00, 0, 0);
}

//	The activate command (0x50) followed by 0x73 makes the following commands writable.
//  • R_RX_PL_WID
//  • W_ACK_PAYLOAD
//  • W_TX_PAYLOAD_NOACK
void nrf_cmd_activate(void)
{
	pin_write(CSN.port, CSN.pin, LOW);
	
	spi_write(0x50); 
		
	spi_write(0x73);
	
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH); 
}

void nrf_set_tx_addr(uint32_t addr_high, uint8_t addr_low, uint8_t auto_ack)
{
	pin_write(CSN.port, CSN.pin, LOW);  //start SPI comms by a LOW on CSN
		
	spi_write(0x20 | 0x10);
		

	/*  5 byte address is devided into a uint8_t low byte
	 *  and a uint32_t high byte
	 *  since the SPI can only send 1 byte at a time
	 *  we first send the low byte
	 *  and then extract the other bytes from the uint32 
	 *  and send them one by one LSB first
	 */
	
	spi_write(addr_low); 

	spi_write(addr_high & 0xFF);

	spi_write((addr_high >> 8) & 0xFF);

	spi_write((addr_high >> 16) & 0xFF);

	spi_write((addr_high >> 24) & 0xFF);
	
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH);
	
	/* If auto ack is enabled then the same address that was written 
	 * to TX_ADDR above must also be written to PIPE 0 because that
	 * is the pipe that it will receive the auto ack on. This cannot
	 * be changed it is hardwaired to receive acks on pipe 0 */
	
	if (auto_ack)
	{	
		nrf_write_reg_specific_bit(0x01, 0, 1); //enable auto ack on pipe 0	
		pin_write(CSN.port, CSN.pin, LOW); 

		//write address into pipe 0
	  spi_write(0x20 | 0x0A);

		spi_write(addr_low); 

		spi_write(addr_high & 0xFF);

		spi_write((addr_high >> 8) & 0xFF);

		spi_write((addr_high >> 16) & 0xFF);

		spi_write((addr_high >> 24) & 0xFF);

		delayUs(CSN_DELAY_TIME);
		pin_write(CSN.port, CSN.pin, HIGH);  //end spi
	}	
}

void nrf_set_rx_addr(uint8_t rx_pipe, uint32_t addr_high, uint8_t addr_low) 
{	
	if (rx_pipe > 1) // because pipe 0 and 1 just need to written directly with 5 bytes
	{
			//RX_ADDR_P# is offest by 10 = RX_ADDR_OFFSET with the pipe number
			//NRF_cmd_write_entire_reg((rx_pipe + RX_ADDR_OFFSET), addr_low);  //for pipe 2 to 5
			nrf_write_reg(rx_pipe + 10, addr_low);
	}
	else
	{
		pin_write(CSN.port, CSN.pin, LOW);

		//write address into pipe 0
	  spi_write(0x20 | (rx_pipe + 10));

		spi_write(addr_low); 

		spi_write(addr_high & 0xFF);

		spi_write((addr_high >> 8) & 0xFF);

		spi_write((addr_high >> 16) & 0xFF);

		spi_write((addr_high >> 24) & 0xFF);
		
		delayUs(CSN_DELAY_TIME);
		pin_write(CSN.port, CSN.pin, HIGH); //end spi
	}
}


void nrf_read_rx_payload(uint8_t *payload)
{
	uint8_t payload_width = nrf_read_reg(0x60);
	
	pin_write(CE.port, CE.pin, LOW);//bring CE LOW to stop listening
	
	pin_write(CSN.port, CSN.pin, LOW); //start SPI
	
	spi_write(0x61); //command that tells the NRF to send the data in the RX_PLD register to us

	for(int i = 0; i < payload_width; i++)
	{
		spi_write(0x61); //idk if this value has to be 0x61 or if its just a dumby byte in order to read 
		payload[i] = spi_read();
	}
	
	pin_write(CE.port, CE.pin, HIGH); //end SPI
	
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH);//bring CE HIGH to start listening again
}

void nrf_write_tx_payload(uint8_t *data, uint8_t length)
{	
	pin_write(CE.port, CE.pin, LOW); //bring CE LOW to stop listening
	
	pin_write(CSN.port, CSN.pin, LOW); //start SPI
	
	spi_write(0xA0); //send the W_TX_PAYLOAD command     

	for(int i = 0; i < length; i++)
	{
		spi_write(data[i]);
	}
	 
	delayUs(CSN_DELAY_TIME);
	pin_write(CSN.port, CSN.pin, HIGH); //end SPI

	//CE set high to start transmition if in TX mode
	// must be held high for a bit then back low
	pin_write(CE.port, CE.pin, HIGH);
	delayUs(20); //this was here for debugging purposes feel free to delete or insert your own
	pin_write(CE.port, CE.pin, LOW); 
	
	nrf_cmd_get_status();
	
}

char nrf_send(uint8_t *payload, uint8_t length)
{
	uint8_t status = 0;
	
	nrf_write_tx_payload(payload, length);
	delayMs(10);
	status = nrf_cmd_get_status();
	
	if(status & 0x20)
	{
		nrf_cmd_clear_interrupts();
		nrf_cmd_flush_tx();
		delayMs(10);
		return 1;
	}
	else if(status & 0x10)
	{
		nrf_cmd_clear_interrupts();
		nrf_cmd_flush_tx();
		delayMs(10);
		return 0;
	}
	else if(status & 0x01) 
	{
		nrf_cmd_flush_tx();
		delayMs(10);
		return 0;
	}
	else
	{
		delayMs(100);
		return 0;
	}
}

char nrf_data_available(void)
{
	if((nrf_cmd_get_status() & RX_DR)){
		nrf_cmd_clear_interrupts();
		return 1;
	}
	else{
		nrf_cmd_clear_interrupts();
		return 0;
	}
}

void nrf_read(uint8_t *dataBuffer)
{
	nrf_read_rx_payload(dataBuffer);
		
	nrf_cmd_flush_rx();
		
	nrf_cmd_listen();
}

