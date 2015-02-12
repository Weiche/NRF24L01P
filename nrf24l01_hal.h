/*
 * nrf24l01_hal.h
 *
 *  Created on: 2014Äê9ÔÂ6ÈÕ
 *      Author: Administrator
 */

#ifndef NRF24L01_HAL_H_
#define NRF24L01_HAL_H_
#include <stdint.h>
#include <stdio.h>
typedef struct{
	int (*Printf) (const char *__restrict __format, ...);
	int (*DelayMs) (const int ms);
}NRF24_OS_Functions_t;
extern NRF24_OS_Functions_t NRF24_OS_Functions;
#define NRF24_Printf   NRF24_OS_Functions.Printf
#define NRF24_DelayMs  NRF24_OS_Functions.DelayMs

extern void NRF24_CE_Enable(void);
extern void NRF24_CE_Disable(void);
int NRF24_HAL_Init(void);
uint32_t NRF24_HAL_Test( void );

#endif /* NRF24L01_HAL_H_ */
