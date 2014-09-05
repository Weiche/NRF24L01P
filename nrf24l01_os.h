#ifndef __NRF24_OS_H
#define __NRF24_OS_H
/* from hal */
extern void NRF24_CE_Enable(void);
extern void NRF24_CE_Disable(void);
#include <stdio.h>

#define NRF24_Printf	printf
#define NRF24_DelayMs(ms)

void NRF24_HAL_Init(void);
uint32_t NRF24_HAL_Test( void );
#endif
