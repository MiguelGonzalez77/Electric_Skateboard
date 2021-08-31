// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
//                     THIS DRIVER DOES NOT ALLOW THE USE OF THE IRQ PIN
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef NRF_Driver_H_
#define NRF_Driver_H_

#include  <stdint.h>
#include <stdbool.h>
#include "GPIO_Driver.h"

//=====================================================================================================
//													NRF COMMANDS
//=====================================================================================================

#define R_REGISTER					0x00  
#define W_REGISTER					0x20  
#define R_RX_PAYLOAD				0x61
#define W_TX_PAYLOAD				0xA0
#define FLUSH_TX					0xE1
#define FLUSH_RX					0xE2
#define REUSE_TX_PL					0xE3 
#define R_RX_PL_WID					0x60
#define W_ACK_PAYLOAD				0xA8
#define NOP							0xFF
#define W_TX_PAYLOAD_NACK        	0xB0
#define ACTIVATE					0x50
#define ACTIVATE_BYTE     			0x73  


//=====================================================================================================
//								NRF REGISTERS AND SPECIFIC BITS
//=====================================================================================================

#define REGISTER_MASK 0x1F
#define NRF_CONFIG  	0x00
#define EN_AA       	0x01
#define EN_RXADDR   	0x02
#define SETUP_AW    	0x03
#define SETUP_RETR  	0x04
#define RF_CH       	0x05
#define RF_SETUP    	0x06
#define NRF_STATUS  	0x07
#define OBSERVE_TX  	0x08
#define CD          	0x09
#define RX_ADDR_P0  	0x0A
#define RX_ADDR_P1  	0x0B
#define RX_ADDR_P2  	0x0C
#define RX_ADDR_P3  	0x0D
#define RX_ADDR_P4  	0x0E
#define RX_ADDR_P5  	0x0F
#define TX_ADDR     	0x10
#define RX_PW_P0    	0x11
#define RX_PW_P1    	0x12
#define RX_PW_P2    	0x13
#define RX_PW_P3    	0x14
#define RX_PW_P4    	0x15
#define RX_PW_P5    	0x16
#define FIFO_STATUS 	0x17
#define DYNPD       	0x1C
#define FEATURE     	0x1D


//=====================================================================================================
//																						NRF BITS
//=====================================================================================================

//CONFIG Register : CONFIG
#define MASK_RX_DR  6
#define MASK_TX_DS  5
#define MASK_MAX_RT 4
#define EN_CRC      3
#define CRCO        2
#define PWR_UP      1
#define PRIM_RX     0

//ENABLE AUTO ACK Register : EN_AA
#define ENAA_P5     5
#define ENAA_P4     4
#define ENAA_P3     3
#define ENAA_P2     2
#define ENAA_P1     1
#define ENAA_P0     0

//Enabled RX Addresses Register : EN_RXADDR
#define ERX_P5      5
#define ERX_P4      4
#define ERX_P3      3
#define ERX_P2      2
#define ERX_P1      1
#define ERX_P0      0

//Setup of Address Widths Register : SETUP_AW
#define AW          0

//Setup of Automatic Retransmission Register : SETUP_RETR
#define ARD         4
#define ARC         0

//RF Channel Register :RF_CH
#define RF_CH_BITS  0

//RF Setup Register : RF_SETUP
#define PLL_LOCK    4
#define RF_DR       3
#define RF_PWR      6
#define LNA_HCURR   0

//STATUS register : STATUS
#define RX_DR       0x40
#define TX_DS       5
#define MAX_RT      4
#define RX_P_NO     1
#define TX_FULL     0

//Transmit observe register : OBSERVE_TX
#define PLOS_CNT    4
#define ARC_CNT     0
// CD
#define CD_BIT      0

//FIFO Status Register : FIFO_STATUS
#define TX_REUSE    6
#define FIFO_FULL   5
#define TX_EMPTY    4
#define RX_FULL     1
#define RX_EMPTY    0

//Enable dynamic payload length Register : DYNPD
#define DPL_P5	    5
#define DPL_P4	    4
#define DPL_P3	    3
#define DPL_P2	    2
#define DPL_P1	    1
#define DPL_P0	    0

//Feature Register : FEATURE
#define EN_DPL	    2
#define EN_ACK_PAY  1
#define EN_DYN_ACK  0

#define PIPE_0	0x00 
#define PIPE_1	0x01
#define PIPE_2	0x02
#define PIPE_3	0x03
#define PIPE_4	0x04
#define PIPE_5	0x05

//offsets that help with writing to registers
#define RX_ADDR_OFFSET  0x0A
#define RX_PW_OFFSET    0x11

//SETUP_AW byte codes
#define THREE_BYTES 0b01
#define FOUR_BYTES  0b10
#define FIVE_BYTES  0b11
#define ENCODING_SCHEME_1_BYTE  0x00
#define ENCODING_SCHEME_2_BYTE  0x01
#define DUMMYBYTE  0xF1




//*****************************************************************************************************
void nrf_init_pins(GPIO_TypeDef *CE_port, uint32_t CE_pin, GPIO_TypeDef *CSN_port, uint32_t CSN_pin);

//		The nrf_init_pins function configures the CE and CSN pins of the NRF module and inits the SPI
//*****************************************************************************************************




//*****************************************************************************************************
void nrf_write_reg(char reg, char data);
//		The nrf_write_reg function allows you to write to an 8 bit register in the NRF24 module
//*****************************************************************************************************


//*****************************************************************************************************
uint8_t nrf_read_reg(char reg);
//		The nrf_read_reg function returns the bits of an 8 bit register in the NRF24 module
//*****************************************************************************************************



//*****************************************************************************************************
void nrf_init_transmiter(uint32_t addr_high, uint8_t addr_low, uint8_t channel, char en_auot_ack, char en_dpl);

//		The nrf_init_transmiter function writes the appropriate bits to the NRF24 for transmit mode.
//    The user must pass into the function the last 4 bits of the address to addr_high, and the first bit of the
//		address to addr_low. The user must also send their desired channel, a 1 if they would like to 
//    enable auto ack, and a 1 to enable dynamic payload. The rest of the settings are fixed as follows:
//		
//    CONFIG register = 0x0A (turns the nrf on, dissables interrupts, and sets the nrf into TX mode)
//		EN_AA register = 0x3F (enables auto ack for all the pipes by default)
//		SETUP_AW register = 0x03 (sets address width to 5)
//		SETUP_RETR register = 0xFF (retransmits the data up to 15 times on a delay of 4000uS)
//		RF_SETUP register = 0x0F (sets the air data rate to 2Mbps and the output power to 0dBm)
//
//*****************************************************************************************************



//*****************************************************************************************************
void nrf_init_reciever(uint8_t pipe, uint32_t addr_high, uint8_t addr_low, uint8_t channel, char en_auto_ack, char en_dpl, uint8_t dpl_w);

//		The nrf_init_reciever function writes the appropriate bits to the NRF24 for recieve mode.
//		The user must pass into the function their desired pipe ([1,5], pipe 0 is reserved for TX mode).
//		The user must also pass the last 4 bits of the address to addr_high, and the first bit of the
//		address to addr_low. The user must also send their desired channel, a 1 if they would like to 
//    enable auto ack, and a 1 to enable dynamic payload. The rest of the settings are fixed as follows:
//
//    CONFIG register = 0x7B (turns the nrf on, dissables interrupts, and sets the nrf into RX mode)
//		EN_AA register = 0x3F (enables auto ack for all the pipes by default)
//		SETUP_AW register = 0x03 (sets address width to 5)
//		SETUP_RETR register = 0xFF (retransmits the data up to 15 times on a delay of 4000uS)
//		RF_SETUP register = 0x0F (sets the air data rate to 2Mbps and the output power to 0dBm)
//
//*****************************************************************************************************


//****************************************************************************************************
void nrf_write_reg_specific_bit(uint8_t reg, uint8_t bit, uint8_t value);

//		The nrf_write_reg_specific_bit allows the user to change just one bit in their register of choice
//****************************************************************************************************


//****************************************************************************************************
void nrf_read_reg_multiple_bytes(uint8_t reg, uint8_t *data_buff);

//		The nrf_read_reg_multiple_bytes allows the user to read a reister with more than one byte
//		Such as the address registers
//****************************************************************************************************



//****************************************************************************************************
void nrf_cmd_listen(void);
															
//		The nrf_cmd_listen function simply pulls the CE pin high
//****************************************************************************************************																



//****************************************************************************************************
uint8_t nrf_cmd_get_status(void);

//		The nrf_cmd_get_status function returns the value in the status register
//****************************************************************************************************



//****************************************************************************************************
void nrf_cmd_clear_interrupts(void);
													
//		The nrf_cmd_clear_interrupts function clears the interrupts in the NRF module 
//****************************************************************************************************




//****************************************************************************************************
void nrf_cmd_flush_tx(void);

//		The nrf_cmd_flush_tx function clears the TX_FIFO value
//****************************************************************************************************



//****************************************************************************************************
void nrf_cmd_flush_rx(void);

//		The nrf_cmd_flush_rx function clears the RX_FIFO register 
//****************************************************************************************************



//****************************************************************************************************
void nrf_cmd_act_as_RX(void);
															
//		The nrf_cmd_act_as_RX function puts the NRF in RX mode
//****************************************************************************************************



//****************************************************************************************************
void nrf_cmd_act_as_TX(void);

//		The nrf_cmd_act_as_TX function puts the NRF in TX mode
//****************************************************************************************************



//****************************************************************************************************
void nrf_cmd_activate(void);

//		The nrf_cmd_activate function enables the use of the following features: 
//			• R_RX_PL_WID
//			• W_ACK_PAYLOAD
//			• W_TX_PAYLOAD_NOACK
//    Note that this function only enables the use of these features. 
//****************************************************************************************************



//****************************************************************************************************
void nrf_set_tx_addr(uint32_t addr_high, uint8_t addr_low, uint8_t auto_ack);

//		The nrf_set_tx_addr function sets the address for the transmitter given the last 4 
//    bytes (addr_high) and the first byte (addr_low) of the address. You can also send
//		a 1 for the auto_ack to enable auto ack on the transmitter.
//****************************************************************************************************


//****************************************************************************************************
void nrf_set_rx_addr(uint8_t rx_pipe, uint32_t addr_high, uint8_t addr_low);

//		The nrf_set_rx_addr function sets the address for the reciever given the last 4 
//		bytes (addr_high) and the first byte (addr_low) of the address. You must also specify
//		the pipe of the reciever [1-5] 
//****************************************************************************************************



//****************************************************************************************************
void nrf_read_rx_payload(uint8_t *payload);

//		The nrf_read_rx_payload function reads the paylod from the RX and puts the value into 
//		a pointer
//****************************************************************************************************



//****************************************************************************************************
void nrf_write_tx_payload(uint8_t *data, uint8_t len);

//		The nrf_write_tx_payload function writes the given payload [*data] to the transmitter
//****************************************************************************************************



//****************************************************************************************************
char nrf_send(uint8_t *data, uint8_t length);

//		The nrf_send function sends the payload [*data] to a reciever. Returns a 1 if succssfully sent 
//****************************************************************************************************



//****************************************************************************************************
char nrf_data_available(void);

//		The nrfh_data_available function returns a 1 if there is a payload in the RX
//****************************************************************************************************



//****************************************************************************************************
void nrf_read(uint8_t *dataBuffer);

//		The nrf_read function reads the payload from the RX and puts the value into a pointer
//****************************************************************************************************



#endif

