#include <stdint.h>
#include "nrf24_config.h"

#include "lpc8xx.h"
#include "lpc8xx_spi.h"



void NRF24_Enable(void) {
	
	LPC_GPIO_PORT->CLR0 = NRF24_CSN_PIN;
	#if NRF24_CSN_DELAY
	{
	uint16_t i;
	for( i = NRF24_CSN_DELAY; i!= 0; i-- );
	}
	#endif
}
void NRF24_Disable(void) {
	
	#if NRF24_CSN_DELAY
	uint16_t i;
	for( i = NRF24_CSN_DELAY; i!= 0; i-- );
	#endif
	LPC_GPIO_PORT->SET0 = NRF24_CSN_PIN;
	#if NRF24_CSN_DELAY
	for( i = NRF24_CSN_DELAY; i!= 0; i-- );
	#endif
}

void NRF24_CE_Enable(void) {
	LPC_GPIO_PORT->SET0 = NRF24_CE_PIN;
}
void NRF24_CE_Disable(void) {
	LPC_GPIO_PORT->CLR0 = NRF24_CE_PIN;
}

void NRF24_HAL_Init(void) {
	  uint8_t cfg;
    /* Init CE CSN*/
    LPC_GPIO_PORT->DIR0 |= NRF24_CE_PIN | NRF24_CSN_PIN ;	
	NRF24_CE_Disable();
	NRF24_Disable();
	/* Enable SPI clock */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);
	/* Peripheral reset control to SPI, a "1" bring it out of reset. */
	LPC_SYSCON->PRESETCTRL &= ~(0x1<<0);
	LPC_SYSCON->PRESETCTRL |= (0x1<<0);
	
	cfg = CFG_MASTER ;
    LPC_SPI0->CFG = (cfg & ~CFG_ENABLE);
	LPC_SPI0->CFG |= CFG_ENABLE;
	
	LPC_SPI0->DIV = NRF24_SPI_DIV;

}

void NRF24_InsWrite(uint8_t instruction,const uint8_t *pbuff,uint32_t num){
	NRF24_Enable();

	/* transfer instruction*/
	while((LPC_SPI0->STAT& STAT_TXRDY) == 0);
	LPC_SPI0->TXDATCTL = TXDATCTL_FSIZE(8-1) | TXDATCTL_EOT | TXDATCTL_RX_IGNORE | instruction;
	
	/* transfer data */
	while( num != 0 ){
		while((LPC_SPI0->STAT& STAT_TXRDY) == 0);
		LPC_SPI0->TXDATCTL = TXDATCTL_FSIZE(8-1) | TXDATCTL_EOT | TXDATCTL_RX_IGNORE | *pbuff++;
		num--;
	}
	
	NRF24_Disable();
}

void NRF24_InsRead(uint8_t instruction,uint8_t *pbuff,uint32_t num){
	uint16_t timeout ;
	NRF24_Enable();
	
	/* transfer instruction*/
	while((LPC_SPI0->STAT& STAT_TXRDY) == 0);
	LPC_SPI0->TXDATCTL = TXDATCTL_FSIZE(8-1) | TXDATCTL_EOT | TXDATCTL_RX_IGNORE | instruction;
	
	/* transfer data */
	while( num != 0 ){
		
		while((LPC_SPI0->STAT& STAT_TXRDY) == 0);
		LPC_SPI0->TXDATCTL = TXDATCTL_FSIZE(8-1) | TXDATCTL_EOT | 0xFF;
		
		timeout = 0xFFFF;
		while((LPC_SPI0->STAT& STAT_RXRDY) == 0 && timeout-- );
		*pbuff++ = LPC_SPI0->RXDAT;
		
		num--;
	}
	
	NRF24_Disable();
}

uint32_t NRF24_HAL_Test( void ){
	uint8_t read;
	uint8_t write = 0x01 | 0x08;
	NRF24_HAL_Init();
	NRF24_InsWrite(0x20|0x00 ,&write,1);
	NRF24_InsRead(0x00,&read,1);
	if( read == (0x01 | 0x08) ){
		return 0;
	}else{
		return 1;
	}

}

#if TEST
void main( void ){
	NRF24_Test();
}
#endif
