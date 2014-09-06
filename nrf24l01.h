#ifndef NRF24L01_H
#define NRF24L01_H
#ifndef U8_MAX
#include <stdint.h>
#endif
//********************************************************************************************************************//
// SPI(nRF24L01) registers(addresses)

#define CONFIG          0x00  // 'Config' register address
#define EN_AA           0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR       0x02  // 'Enabled RX addresses' register address
#define SETUP_AW        0x03  // 'Setup address width' register address
#define SETUP_RETR      0x04  // 'Setup Auto. Retrans' register address
#define RF_CH           0x05  // 'RF channel' register address
#define RF_SETUP        0x06  // 'RF setup' register address
#define STATUS          0x07  // 'Status' register address
#define OBSERVE_TX      0x08  // 'Observe TX' register address
#define CD              0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0      0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1      0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2      0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3      0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4      0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5      0x0F  // 'RX address pipe5' register address
#define TX_ADDR         0x10  // 'TX address' register address
#define RX_PW_P0        0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1        0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2        0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3        0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4        0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5        0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS     0x17  // 'FIFO Status Register' register address
#define DYPLD			0x1C
#define FEATURE			0x1D

#define RF_SETUP_RF_DR		(1<<3)
#define RF_SETUP_PWR_0DB	(3<<1)
#define RF_SETUP_PWR_7DB	(0x07)
#define RF_SETUP_LNA_HCURR	(0x01)

//********************************************************************************************************************//
// SPI(nRF24L01) commands

#define R_REG			0x00  // Define read command to register
#define W_REG       	0x20  // Define write command to register
#define RD_RX_PLOAD     0x61  // Define RX payload register address
#define WR_TX_PLOAD     0xA0  // Define TX payload register address
#define FLUSH_TX        0xE1  // Define flush TX register command
#define FLUSH_RX        0xE2  // Define flush RX register command
#define REUSE_TX_PL     0xE3  // Define reuse TX payload register command
#define NOP             0xFF  // Define No Operation, might be used to read status register
/* Hal */
#define NRF24_HAL_ERR	1
#define NRF24_HAL_OK  0

#define NRF24_OK  0


/* Nrf config structure*/

typedef struct {
	uint8_t Address_Length;
#define Address_Length_5Byte 5
#define Address_Length_4Byte 4
#define Address_Length_3Byte 3

	uint8_t Auto_Retransmit_Delay;
#define AUTO_RETRANSMIT_DELAY_MIN	0
#define AUTO_RETRANSMIT_DELAY_MAX	0xF
	/*
	 250 - 4000 us for 0 - 0xF
	 */
	uint8_t Auto_Retransmit_Count;
	/* Up to 15 Re-Transmit on fail of AA 0xF*/

	uint8_t Channel;
	/*Channel 0x00-0x60*/
	uint8_t AutoAck_Enable_Flag;
	/* (1<<n) to enable EN_AA of pipe n :*/

	uint8_t DataRate;
#define DataRate_1M			(0)
#define DataRate_2M			(RF_SETUP_RF_DR)
#define DataRate_250K		(1<<5)

	uint8_t Interrupt_Mask;
#define Interrupt_Mask_RX_DR		(1<<6)
#define Interrupt_Mask_TX_DS		(1<<5)
#define Interrupt_Mask_MAX_RT		(1<<4)
#define Interrupt_Mask_All			(7<<4)

	uint8_t CRC_Control; //Enable CRC. Forced high if one of the bits inthe EN_AA is high
#define CRC_Control_Enable			(1<<3)
#define CRC_Control_Disable			(0<<3)

	uint8_t CRC_Length;
#define CRC_Length_1Byte	(0<<2)
#define CRC_Length_2Byte	(1<<2)

	uint8_t Power;
#define Power_On			(1<<1)
#define Power_Off			(0)

	uint8_t TX_Power;
#define TX_Power_0DB	RF_SETUP_PWR_0DB
#define TX_Power_7DB	RF_SETUP_PWR_7DB
	uint8_t Mode;
#define Mode_RX				(1)
#define Mode_TX				(0)
} NRF24_InitTypedef;

#define STATUS_RX_DR		(1<<6)
#define STATUS_TX_DS		(1<<5)
#define STATUS_MAX_RT		(1<<4)

#define STATUS_INT_ALL		(7<<4)

#define STATUS_RX_P_NO		(7<<1)
#define STATUS_TX_FULL		(1<<0)

typedef struct{
	int (*NRF24_Printf) (__const char *__restrict __format, ...);
	int (*NRF24_DelayMs) (__const int ms);
}NRF24_OS_Functions_t;
extern NRF24_OS_Functions_t NRF24_OS_Functions;
#define NRF24_Printf   (NRF24_OS_Functions.NRF24_Printf)
#define NRF24_DelayMs  (NRF24_OS_Functions.NRF24_DelayMs)
void NRF24_Dump(void) ;
void NRF24_Send(NRF24_InitTypedef *nrf, const uint8_t *pbuff, const uint16_t length) ;
void NRF24_Receive(NRF24_InitTypedef *nrf, uint8_t *pbuff, uint32_t *length);
void NRF24_Init(NRF24_InitTypedef *nrf) ;
void NRF24_Get_TX_Addr(uint8_t *tx_addr);
void NRF24_Set_TX_Addr(const uint8_t *tx_addr) ;
void NRF24_Get_RX_Addr(uint8_t *rx_addr, uint8_t pipe);
void NRF24_Set_RX_Addr(const uint8_t *rx_addr, uint8_t pipe);
void NRF24_TXFIFO_Write(const uint8_t *pbuff, uint32_t num);
void NRF24_SetRX_Length(uint8_t pipe,uint32_t length);
void NRF24_Write_Reg(uint8_t reg, uint8_t data);
uint8_t NRF24_Read_Reg(uint8_t reg);
/* raw api */
uint8_t NRF24_Reg_SetBit(uint8_t reg, uint8_t value);
uint8_t NRF24_Reg_ResetBit(uint8_t reg, uint8_t value);
#define NRF24_SetRXMode() NRF24_Reg_SetBit(CONFIG,0x01)
#define NRF24_SetTXMode() NRF24_Reg_ResetBit(CONFIG,0x01)
/* os */
extern void NRF24_CE_Enable(void);
extern void NRF24_CE_Disable(void);
void NRF24_HAL_Init(void);
uint32_t NRF24_HAL_Test( void );

#endif
