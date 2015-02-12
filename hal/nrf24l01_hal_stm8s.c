#include "nrf24l01_os.h"
#include "stm8s.h"
#include "stm8s_spi.h"

#define NRF24_CSN_PIN   GPIO_PIN_2
#define NRF24_CSN_PORT  GPIOD
#define NRF24_Enable()  NRF24_CSN_PORT->ODR &= ~NRF24_CSN_PIN;
#define NRF24_Disable()  NRF24_CSN_PORT->ODR |= NRF24_CSN_PIN;

static void NRF24_CE_Init(void){


}
void NRF24_HAL_Init(void) {
    SPI_Init(SPI_FIRSTBIT_MSB, 
             SPI_BAUDRATEPRESCALER_4, 
             SPI_MODE_MASTER, 
             SPI_CLOCKPOLARITY_LOW, 
             SPI_CLOCKPHASE_1EDGE,
             SPI_DATADIRECTION_2LINES_FULLDUPLEX, 
             SPI_NSS_SOFT,
             0);
    SPI_Cmd(ENABLE);
    NRF24_CE_Init();
    GPIO_Init(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_MODE_OUT_PP_HIGH_FAST); 
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_HIGH_FAST);
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);
    NRF24_Disable();

}
uint8_t NRF24_WriteRead( uint8_t data ){
    while(!(SPI->SR & SPI_SR_TXE));
    SPI->DR = data;
    while(!(SPI->SR & SPI_SR_RXNE));
    return SPI->DR;
}
uint8_t NRF24_Read_Status(void){
    register uint8_t ret;
    NRF24_Enable();
    ret = NRF24_WriteRead(0xFF);
    NRF24_Disable();
    return ret;
}

void NRF24_CE_Enable(void) {
    return;
}
void NRF24_CE_Disable(void) {
    return;
}

void NRF24_InsWrite(uint8_t instruction,const uint8_t *pbuff,uint32_t num){
    NRF24_Enable();
    
    NRF24_WriteRead( instruction ); 
    
    while( num-- ){
        while(!(SPI->SR & SPI_SR_TXE));
        SPI->DR = *pbuff++;
        while(!(SPI->SR & SPI_SR_RXNE));
        instruction = SPI->DR;    
    }
    
    NRF24_Disable();
}
void NRF24_InsRead(uint8_t instruction,uint8_t *pbuff,uint32_t num){
    NRF24_Enable();    
    
    NRF24_WriteRead( instruction );
    
    while( num-- ){
        while(!(SPI->SR & SPI_SR_TXE));
        SPI->DR = 0xFF;
        while(!(SPI->SR & SPI_SR_RXNE));
        *pbuff++ = SPI->DR;    
    }
    
    NRF24_Disable();
}

void NRF24_Test( void ){
	static  uint8_t read;
	uint8_t write = 0x02;
	NRF24_HAL_Init();
        
    NRF24_Enable();
    
    while(!(SPI->SR & SPI_SR_TXE));
    SPI->DR = 0xFF;
    while(!(SPI->SR & SPI_SR_RXNE));
    read = SPI->DR;
    NRF24_Disable();
    NRF24_InsRead(0x00,&read,1);
    NRF24_InsWrite(0x20|0x05 ,&write,1);
    NRF24_InsRead(0x05,&read,1);
    if( read == 0x01 ){
        nop();
    }
    return;
}
#if TEST
void main( void ){
	NRF24_Test();
}
#endif
