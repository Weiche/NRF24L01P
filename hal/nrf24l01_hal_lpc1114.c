#include "lpc11xx.h"
#include <stdint.h>
#include "nrf24_config.h"





void NRF24_Enable(void) {
	
	LPC_GPIO0->DATA &= ~NRF24_CSN_PIN;
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
	LPC_GPIO0->DATA |= NRF24_CSN_PIN;
	#if NRF24_CSN_DELAY
	for( i = NRF24_CSN_DELAY; i!= 0; i-- );
	#endif
}

void NRF24_CE_Enable(void) {
	LPC_GPIO0->DATA |= NRF24_CE_PIN;
}
void NRF24_CE_Disable(void) {
	LPC_GPIO0->DATA &= ~NRF24_CE_PIN;
}
#define SSP_8BIT		(0x07)
#define SSP_SPI_MODE	(0)

#define SSP_CPOL_HIGH	(1<<6)
#define SSP_CPOL_LOW	(0)

#define SSP_CPHA_FIRST	(0)
#define SSP_CPHA_SECOND	(1<<7)

#define SSP_CLKDIV(a)	((a&0xFF)<<8)

#define SSP_CR1_LB	(1<<0)
#define SSP_CR1_SSE	(1<<1)
#define SSP_CR1_MS	(1<<2)

#define SSP_CR1_ENABLE	SSP_CR1_SSE
#define SSP_CR1_MASTER	(0)
#define SSP_SR_TFE	(1<<0)
#define SSP_SR_TNF	(1<<1)
#define SSP_SR_RNE	(1<<2)
#define SSP_SR_RFF	(1<<3)
#define SSP_SR_BYS	(1<<4)

void NRF24_HAL_Init(void) {
	/* Enable AHB clock to the IOCON. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);
	/* Enable AHB clock to the GPIO domain. */
	LPC_SYSCON->SYSAHBCLKCTRL |= ( 1 << 6 );
	/* Enable AHB clock to the SSP0. */
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);	
    /* Init CE CSN*/
    LPC_GPIO0->DIR |= NRF24_CE_PIN | NRF24_CSN_PIN ;	
	NRF24_CE_Disable();
	NRF24_Disable();
	
	/* MOSI */
	LPC_IOCON->PIO0_8 &= ~(0x07);
	LPC_IOCON->PIO0_8 |= (1<<0);
	/* MISO */
	LPC_IOCON->PIO0_9 &= ~(0x07);
	LPC_IOCON->PIO0_9 |= (1<<0);
	/* SCK */
	LPC_IOCON->SCK_LOC = 0x02; /*Select P0_6 as SCK pin*/
	LPC_IOCON->PIO0_6 &= ~(0x07);
	LPC_IOCON->PIO0_6 |= (1<<1);/* set p0_6 SCK */
	

	LPC_SYSCON->SSP0CLKDIV = 1;//
	LPC_SYSCON->PRESETCTRL |= (1<<0);/* reset SPI */
	
	LPC_SSP0->CR0 = SSP_8BIT | SSP_SPI_MODE | SSP_CPOL_LOW | SSP_CPHA_FIRST | SSP_CLKDIV(2);
	LPC_SSP0->CPSR = 0x04;
	/* SPI rate is PCLK 48MHz / (CPSR * (1+SSP_CLKDIV)) = 48/(2*3) = 8MHz*/
	
	LPC_SSP0->CR1 = SSP_CR1_ENABLE | SSP_CR1_MASTER;

}
void __SSP0_Flush(void){
	static volatile char dummy;
	while((LPC_SSP0->SR & SSP_SR_TFE) == 0);/* wait for tx fifo empty */
	while((LPC_SSP0->SR & SSP_SR_RNE)){
		dummy = LPC_SSP0->DR;
	}
}
uint8_t NRF24_SPI_Byte(uint8_t write){
	/* transfer instruction*/
	while( !(LPC_SSP0->SR & SSP_SR_TNF) );
	LPC_SSP0->DR = write;
	while( !(LPC_SSP0->SR & SSP_SR_RNE) );
	write = (uint8_t)(LPC_SSP0->DR);
	return write;	
}
void NRF24_InsWrite(uint8_t instruction,const uint8_t *pbuff,uint32_t num){
	
	
#if NRF24_NOFIFO
	NRF24_Enable();
	NRF24_SPI_Byte(instruction);
	
	/* transfer data */
	while( num != 0 ){
		NRF24_SPI_Byte(*pbuff++);
		num--;
	}
#else
	{
		uint32_t rx_num;
		uint32_t tx_num;
		register uint32_t dummy;
		
		tx_num = num;
		rx_num = num + 1;
		__SSP0_Flush();
		NRF24_Enable();
		LPC_SSP0->DR = instruction;
		while( rx_num || tx_num ){
			if((LPC_SSP0->SR & SSP_SR_TNF) && tx_num ){
				LPC_SSP0->DR = *pbuff++;
				tx_num--;
			}
			if((LPC_SSP0->SR & SSP_SR_RNE) && rx_num ){
				dummy = LPC_SSP0->DR;
				rx_num--;
				(void)dummy;
			}
		}
	}
#endif
	NRF24_Disable();
}

void NRF24_InsRead(uint8_t instruction,uint8_t *pbuff,uint32_t num){
	static volatile uint32_t dummy;
	
	
#if NRF24_NOFIFO
	NRF24_Enable();
	/* empty the rx fifo */
	while(LPC_SSP0->SR & SSP_SR_RNE){
		dummy =  LPC_SSP0->DR;	
	}
	
	
	NRF24_SPI_Byte(instruction);

	/* transfer data */
	while( num ){			
		/* transfer instruction*/
		*pbuff++ = NRF24_SPI_Byte(0xFF);
		
		num--;
	}
#else
	{
		volatile uint32_t rx_num;
		volatile uint32_t tx_num;
		
		
		
		__SSP0_Flush();
		NRF24_Enable();
		tx_num = num;
		rx_num = num;
		NRF24_SPI_Byte(instruction);
		while( rx_num || tx_num ){
			if((LPC_SSP0->SR & SSP_SR_TNF) && tx_num ){
				LPC_SSP0->DR = 0xFF;
				tx_num--;
			}
			if((LPC_SSP0->SR & SSP_SR_RNE) && rx_num ){
				*pbuff++ = LPC_SSP0->DR;
				rx_num--;
			}
		}
	}	
#endif
	NRF24_Disable();
}

uint32_t NRF24_HAL_Test( void ){
	uint8_t tx_addr[5] = {1,2,3,4,5};
	static uint8_t dummy[5];
	
	NRF24_HAL_Init();
	NRF24_InsWrite( 0x20 | 0x10, tx_addr, 5);
	NRF24_InsRead ( 0x00 | 0x10, dummy  ,5);
	
	if( tx_addr[0]==dummy[0] &&  tx_addr[1]==dummy[1] && tx_addr[2]==dummy[2] && tx_addr[3]==dummy[3] && tx_addr[4]==dummy[4]){
		return 0;
	}else{
		return 1;
	}

}

#if TEST
void main( void ){
	NRF24_HAL_Test();
}
#endif
