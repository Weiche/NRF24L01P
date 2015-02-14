#include "nrf24l01.h"
#include <string.h>
#include <stdint.h>

extern void NRF24_InsWrite(uint8_t instruction, const uint8_t *pbuff, uint32_t num);
extern void NRF24_InsRead(uint8_t instruction, uint8_t *pbuff, uint32_t num);

int NULL_PRINTF(const char *__restrict __format, ...){
	return -1;
}
int NULL_DELAYMS(const int ms){
	return -1;
}
NRF24_OS_Functions_t NRF24_OS_Functions = {NULL_PRINTF,NULL_DELAYMS};

void NRF24_Instruction(uint8_t ins) {
	NRF24_InsWrite(ins, NULL, 0);
}
void NRF24_Write_Reg(uint8_t reg, uint8_t data) {
	NRF24_InsWrite(W_REG | reg, &data, 1);
}
uint8_t NRF24_Read_Reg(uint8_t reg) {
	uint8_t data;
	NRF24_InsRead( R_REG | reg, &data, 1);
	return data;
}
uint8_t NRF24_Reg_SetBit(uint8_t reg, uint8_t value) {
	uint8_t data;
	data = NRF24_Read_Reg(reg);
	data |= value;
	NRF24_Write_Reg(reg, data);
	return data;
}
uint8_t NRF24_Reg_ResetBit(uint8_t reg, uint8_t value) {
	uint8_t data;
	data = NRF24_Read_Reg(reg);
	data &= ~value;
	NRF24_Write_Reg(reg, data);
	return data;
}
void NRF24_Get_TX_Addr(uint8_t *tx_addr) {
	NRF24_InsRead(R_REG | TX_ADDR, tx_addr, 5);
}
void NRF24_Set_TX_Addr(const uint8_t *tx_addr) {
	NRF24_InsWrite(W_REG | TX_ADDR, tx_addr, 5);
}
void NRF24_Get_RX_Addr(uint8_t *rx_addr, uint8_t pipe) {
	if (pipe > 5)
		return;
	NRF24_InsRead(R_REG | (RX_ADDR_P0 + pipe), rx_addr, 5);
}
void NRF24_Set_RX_Addr(const uint8_t *rx_addr, uint8_t pipe) {
	if (pipe > 5)
		return;
	if( pipe <= 1 ){
		NRF24_InsWrite(W_REG | (RX_ADDR_P0 + pipe), rx_addr, 5);
	}else{
		NRF24_InsWrite(W_REG | (RX_ADDR_P0 + pipe), rx_addr, 1);
	}
}
void NRF24_TXFIFO_Write(const uint8_t *pbuff, uint32_t num) {
	NRF24_InsWrite(WR_TX_PLOAD, pbuff, num <= 32 ? num : 32);
}
void NRF24_TXFIFO_Write_NoACK(const uint8_t *pbuff, uint32_t num) {
	NRF24_InsWrite(WR_TX_PLOAD_NOACK, pbuff, num <= 32 ? num : 32);
}
void NRF24_RXFIFO_Read(uint8_t *pbuff, uint32_t num) {
	NRF24_InsRead(RD_RX_PLOAD, pbuff, num <= 32 ? num : 32);
}
void NRF24_SetRX_Length(uint8_t pipe, uint32_t length) {
	NRF24_Write_Reg(RX_PW_P0 + pipe, length > 32 ? 32 : length);
}

void NRF24_Dump(void) {
	volatile uint8_t value;
	uint8_t addr[5] = { 0, };
	value = NRF24_Read_Reg(CONFIG);
	NRF24_Printf("CONFIG:0x%02X\r\n", value);
	if (value & Mode_RX) {
		NRF24_Printf("RX_Mode    ");
	} else {
		NRF24_Printf("TX_Mode    ");
	}
	if (value & Power_On) {
		NRF24_Printf("Power On    ");
	} else {
		NRF24_Printf("Power Off    ");
	}
	if (value & CRC_Length_2Byte) {
		NRF24_Printf("CRC 2byte    ");
	} else {
		NRF24_Printf("CRC 1byte    ");
	}
	if (value & CRC_Control_Enable) {
		NRF24_Printf("CRC On    ");
	} else {
		NRF24_Printf("CRC Off    ");
	}
	NRF24_Printf("\r\n");
	value = NRF24_Read_Reg(RF_CH);
	NRF24_Printf("RC_CH:0x%02X\r\n", value);

	value = NRF24_Read_Reg(RF_SETUP);
	NRF24_Printf("RF_SETUP:0x%02X\r\n", value);

	value = NRF24_Read_Reg(STATUS);
	NRF24_Printf("STATUS:0x%02X\r\n", value);

	value = NRF24_Read_Reg(EN_AA);
	NRF24_Printf("EN_AA:0x%02X\r\n", value);

	value = NRF24_Read_Reg(EN_RXADDR);
	NRF24_Printf("EN_RXADDR:0x%02X\r\n", value);

	NRF24_Get_TX_Addr(addr);
	NRF24_Printf("TX Addr: %02X%02X%02X%02X%02X\r\n\r\n", addr[0], addr[1], addr[2], addr[3], addr[4]);
	NRF24_Get_RX_Addr(addr, 0);
	NRF24_Printf("RX Pipe 0: %02X%02X%02X%02X%02X\r\n", addr[0], addr[1], addr[2], addr[3], addr[4]);
	NRF24_Get_RX_Addr(addr, 1);
	NRF24_Printf("RX Pipe 1: %02X%02X%02X%02X%02X\r\n", addr[0], addr[1], addr[2], addr[3], addr[4]);

}


void NRF24_SendPrepare(uint8_t *mac_addr, const uint8_t *pbuff, const uint16_t length, const int NoACK) {
	NRF24_CE_Disable();
	/* TX mode */
	NRF24_Reg_ResetBit(CONFIG, 0x01);
	/*	flush fifo ()*/
	NRF24_Flush_TX();
	NRF24_Write_Reg(STATUS,STATUS_MAX_RT|STATUS_TX_DS);
	NRF24_Set_TX_Addr(mac_addr);
	
	if ( !NoACK ){
		NRF24_TXFIFO_Write(pbuff, length);
	}else{
		NRF24_Set_RX_Addr(mac_addr,0);
		NRF24_TXFIFO_Write_NoACK(pbuff, length);
	}	
	NRF24_CE_Enable();
}

int NRF24_SendPolling(void){	
	volatile uint8_t status,fifo_status;
	do{			
		status = NRF24_Read_Reg(STATUS);
		fifo_status = NRF24_Read_Reg(FIFO_STATUS);
		if( status & STATUS_MAX_RT ){				
			goto FAIL;
		}
		if ( status&STATUS_TX_DS ){
			goto OK;
		}
	}while(1);
	
	OK:
	NRF24_CE_Disable();
	return 0;
	
	FAIL:
	NRF24_CE_Disable();
	return -1;
	
}

int NRF24_SendPacketBlocking(uint8_t *mac_addr, const uint8_t *pbuff, const uint16_t length, const int NoACK){	
	NRF24_SendPrepare(mac_addr, pbuff, length, NoACK);
	return NRF24_SendPolling();
}

int NRF24_GetReceivedPacket( uint8_t *pbuff, uint32_t *length, uint8_t *pipe , uint8_t status) {
	uint8_t dummy;
	
	if( status & STATUS_RX_DR ){
		return -1;
	}
	NRF24_InsRead(0x60, &dummy, 1);
	*length = dummy;

	if (*length <= 32 && *length > 0){
		NRF24_RXFIFO_Read(pbuff, *length);
		NRF24_Write_Reg( STATUS, STATUS_RX_DR );
		*pipe = (status & RX_PipeNum_Mask) >> 1;
		return 0;
	}
		return -2;
}


/**
 * Configure radio defaults and turn on the radio in receive mode.
 * This configures the radio to its max-power, max-packet-header-length settings.  If you want to reduce power consumption
 * or increase on-air payload bandwidth, you'll have to change the config.
 */
void NRF24_Init(NRF24_InitTypedef *nrf) {
	uint8_t value;
	NRF24_CE_Disable();
	// set address width 
	NRF24_Write_Reg(SETUP_AW, nrf->Address_Length - 2);

	// set Enhanced Shockburst retry to every 586 us, up to 5 times.  If packet collisions are a problem even with AA enabled,
	// then consider changing the retry delay to be different on the different stations so that they do not keep colliding on each retry.
	value = nrf->Auto_Retransmit_Delay << 8 | nrf->Auto_Retransmit_Count;
	NRF24_Write_Reg(SETUP_RETR, value);

	// Set to use 2.4 GHz channel 110.
	value = nrf->Channel & 0x7F;
	NRF24_Write_Reg(RF_CH, value);

	// Set radio to 2 Mbps and high power.  Leave LNA_HCURR at its default.
	if ((nrf->TX_Power != TX_Power_0DB) && (nrf->TX_Power != TX_Power_7DB)) {
		nrf->TX_Power = TX_Power_0DB;
	}
	value = nrf->DataRate | nrf->TX_Power;
	NRF24_Write_Reg(RF_SETUP, value);

	// clear the interrupt flags in case the radio's still asserting an old unhandled interrupt
	value = STATUS_INT_ALL;	//Write 1 to status reg bits to clear
	NRF24_Write_Reg(STATUS, value);

	NRF24_Write_Reg(EN_AA, nrf->AutoAck_Enable_Flag);
	NRF24_Write_Reg(EN_RXADDR, 0x3F);
	// flush the FIFOs in case there are old data in them.
	NRF24_Instruction(FLUSH_TX);
	NRF24_Instruction(FLUSH_RX);

	// Enable 2-byte CRC and power up in receive mode.
	value = nrf->CRC_Control | nrf->CRC_Length | nrf->Interrupt_Mask | nrf->Power | nrf->Mode;
	NRF24_Write_Reg(CONFIG, value);

	/* Enable noack command default */
	NRF24_Write_Reg(FEATURE,0x01);
}

// scanning all channels in the 2.4GHz band
int NRF24_ScanChannels(uint16_t* channel) {
	uint32_t j, i;
	NRF24_CE_Disable();
	for (j = 0; j < 200; j++) {
		for (i = 0; i < 0x3F; i++) {
			// select a new channel
			NRF24_Write_Reg(RF_CH, (128 * i) / 0x3F);

			// switch on RX
			NRF24_SetRXMode();
			NRF24_CE_Enable();

			// wait enough for RX-things to settle
			NRF24_DelayMs(40);

			// this is actually the point where the RPD-flag
			// is set, when CE goes low
			NRF24_CE_Disable();

			// read out RPD flag; set to 1 if
			// received power > -64dBm
			if (NRF24_Read_Reg(0x09) > 0) {
				channel[i]++;
			}
		}
	}
	return 0;
}
