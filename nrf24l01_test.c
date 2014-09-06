/*
 * nrf24l01_test.c
 *
 *  Created on: 2014Äê9ÔÂ6ÈÕ
 *      Author: Administrator
 */

#include <stdint.h>
#include "nrf24l01.h"

extern void NRF24_Enable(void);
extern void NRF24_Disable(void);
extern void NRF24_CE_Enable(void);
extern void NRF24_CE_Disable(void);
extern void NRF24_InsWrite(uint8_t instruction,const uint8_t *pbuff,uint32_t num);
extern void NRF24_InsRead(uint8_t instruction,uint8_t *pbuff,uint32_t num);

int NRF24_SPI_Test( void ){
	uint8_t read;
	uint8_t write = 0x01;
	NRF24_HAL_Init();
	NRF24_InsWrite(0x20|0x05 ,&write,1);
	NRF24_InsRead(0x00,&read,1);
	NRF24_Printf("Config = 0x%02X\r\n",read);
	if( read == 0x01 ){
		NRF24_Printf("Nrf24 Spi OK!\r\n");
		return 0;
	}else{

		NRF24_Printf("Hal test failed!\r\n");
		return -2;
	}

}

void NRF24_CE_Test(void){
	NRF24_CE_Enable();
	NRF24_CE_Disable();
}

void NRF24_CSN_Test(void){
	NRF24_Enable();
	NRF24_Disable();
}
#if TEST
void main( void ) {
	NRF24_Test();
}
#endif
