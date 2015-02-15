#include "../nrf24l01.h"
#include "../nrf24l01_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
void NRF24_Net_ReceiveCallback(const uint8_t* payload , const uint32_t len){
	uint8_t data[64];
	uint8_t i = 0;

	memcpy( data, payload, 32 );
	for (i = 0; i < len; i++) {
		if (data[i] == 0)
			break;
		else if( data[i] == '\r' || data[i]=='\n')
			putc(data[i], stdout);
		else if (data[i] >= 0x80 || (data[i]<0x20))
			putc('#', stdout);

		else
			putc(data[i], stdout);
	}

}
/*
 @return
 NRF24_HAL_ERR	1
 NRF24_HAL_OK  0
 */
int main(int argc, char **argv) {
	uint32_t data_length;
	uint8_t data_buff[128], rx_addr[5];
	uint32_t i, len;
	uint32_t seq = 0, last_seq = 0, first_seq = 0, current_seq = 0;
	uint32_t seq_lost = 0;
	int result;
	static uint8_t read;
	NRF24_InitTypedef nrf;
	static uint8_t tx_addr[5] = { 0xC2, 0xC2, 0xC2, 0xC2, 0xC2 };
	static uint8_t network_addr[4] = {  2, 3, 4, 5 };
	const uint8_t dummy_data[4] = { 'A', 'B', 'B', 'B' };

	NRF24_HAL_Init();
	nrf.Address_Length = Address_Length_5Byte;
	nrf.Auto_Retransmit_Count = 0;
	nrf.Auto_Retransmit_Delay = 1000 / 250;
	nrf.Channel = 0x0F;
	nrf.CRC_Control = CRC_Control_Disable;
	nrf.CRC_Length = CRC_Length_2Byte;
	nrf.Interrupt_Mask = Interrupt_Mask_All;
	nrf.Power = Power_On;
	nrf.Mode = Mode_RX;
	nrf.DataRate = DataRate_1M;
	nrf.AutoAck_Enable_Flag = 0x00;

	while ((result = getopt(argc, argv, "rS:C:I:")) != -1) {
		switch (result) {
		case 'S': {
			uint32_t speed = 0;
			sscanf(optarg, "%u", &speed);
			if (speed == 250) {
				nrf.DataRate = DataRate_250K;
			} else if (speed == 1000) {
				nrf.DataRate = DataRate_1M;
			} else if (speed == 2000) {
				nrf.DataRate = DataRate_2M;
			} else {
				printf("Invalid speed setting\n");
				return 1;
			}
			printf("NRF24 Speed Setting->%u\n", speed);
			break;
		}
		case 'C':{
			uint32_t channel = 0xFFFF;
			int ret_scanf;
			ret_scanf = sscanf(optarg, "%u", &channel);
			if( channel>0x3F ){
					printf("Invalid Channel Setting\n");
			}else{
				nrf.Channel = channel;
				printf("Set Channel:%u\n",channel);
			}
			break;
		}
		case 'r':{
			nrf.Auto_Retransmit_Count = 15;
			nrf.CRC_Control = CRC_Control_Enable;
			nrf.AutoAck_Enable_Flag = 0x02;
			printf("Enable Enhanced ShockBurst\r\n");
			break;
		}
		case 'I':{
			uint32_t self_ip = 0xFFFF;
			int ret_scanf;
			ret_scanf = sscanf(optarg, "%u", &self_ip);
			if( self_ip <= 255 ){
				Network.self_ip = self_ip;
			}else{
				printf("Invalid IP address\r\n");
			}
			break;
		}
		case ':':
			fprintf(stdout, "%c needs value\n", result);
			return 1;
			break;
		case '?':
			fprintf(stdout, "unknown\n");
			return 1;
			break;

		}
	}

	NRF24_Init(&nrf);
	NRF24_Net_Init( &nrf, network_addr, Network.self_ip);

	NRF24_Dump();
	NRF24_ReceiveStart();
	while(1){
		/* send a message */
		if (NRF24_Read_Reg(FIFO_STATUS) & 0x01){
			NRF24_DelayMs(1);
		}
		NRF24_Net_ReceiveTask();
	}

}
