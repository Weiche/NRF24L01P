#include "../nrf24l01.h"
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

/*Error correction code*/
/*Generate 32byte nrf24 packet */
/*  config */
#define ECC_REPEAT 8
/*  End config */
#if (((ECC_REPEAT+1)/2)!=((ECC_REPEAT)/2))
#error "Repeat must be odd"
#endif

uint32_t ECC_Encode(uint8_t data, uint8_t *wbuff) {
	uint8_t i;
	for (i = 0; i <= ECC_REPEAT; i++) {
		wbuff[i] = data;
	}
	return (ECC_REPEAT + 1);
}

void ECC_Decode(const uint8_t *rbuff, uint8_t* data) {
	uint8_t bit, i, temp, zero, one;

	*data = 0;

	for (bit = 0; bit < 8; bit++) {
		zero = 0;
		one = 0;
		for (i = 0; i <= ECC_REPEAT; i++) {
			temp = rbuff[i] >> bit;
			if (temp & 0x01) {
				one++;
			} else {
				zero++;
			}
		}
		if (one > zero) {
			*data |= (1 << bit);
		}
	}
}


void NRF24_Listen(NRF24_InitTypedef *pnrf, uint8_t pipe, void (*callback)(const uint8_t, const uint8_t*, const uint32_t)) {
#define RX_PipeNum_Mask (0x07<<1)//[3:1]
	static uint8_t data_buff[32];
	uint8_t read, i;
	uint32_t len;

	NRF24_Reg_SetBit(CONFIG, 0x01);
	NRF24_CE_Enable();

	while (1) {
		memset(data_buff, 0, sizeof(data_buff));
		while (1) {
			read = NRF24_Read_Reg(STATUS);
			if (read & STATUS_RX_DR) {
				break;
			}
			if (NRF24_Read_Reg(FIFO_STATUS) & 0x01)
				NRF24_DelayMs(1);
		};
		NRF24_Receive(pnrf, data_buff, &len);
		callback((read & RX_PipeNum_Mask) >> 1, data_buff, len);
	}
}
void NRF24_Callback1(const uint8_t pipe, const uint8_t *pdata, const uint32_t len) {
	int ret;
	static FILE *fp = 0;
	static uint8_t last_h;
	static uint8_t last_t;
	uint32_t h, t, s, count;
	uint8_t temp;
	time_t timer;
	char *ptime_str,*pcursor0;
	const char *pcursor;

	if ((fp = fopen("/var/log/lpc810.log", "rw+a")) == NULL) {
		printf("ファイルのオープンに失敗しました．\n");
		exit(1);
	}
	time(&timer);
	ptime_str = ctime(&timer);
	pcursor = ptime_str;
	while (*pcursor0 != '\n')
		pcursor0++;
	*pcursor0 = 0;

	pcursor = pdata;
	printf("32Byte Packet\r\n-----------------\r\n");
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	pcursor += 4;
	printf("0x%02X 0x%02X 0x%02X 0x%02X \r\n", pcursor[0], pcursor[1], pcursor[2], pcursor[3]);
	printf("End\r\n-----------------\r\n");
	printf("%s\r\n", ptime_str);

	pcursor = pdata;
	ECC_Decode(pcursor, &temp);
	s = temp;
	ECC_Decode(pcursor + 9, &temp);
	h = temp;
	ECC_Decode(pcursor + 18, &temp);
	t = temp;
	printf("S=%u,H=%u,T=%u\r\n", s, h, t);

	if (s == 0xF1 && t <= 50 && h <= 100) {
		count = fprintf(fp, "%d\n%d\n%d\nEnd\n", timer, h, t);
	}

	fclose(fp);
	fp = 0;
	fflush(stdout);
}
void NRF24_Callback2(const uint8_t pipe, const uint8_t *pdata, const uint32_t len) {
	time_t timer;
	static time_t last_called_time;
	static uint32_t counter = 0;
	time(&timer);
	counter++;
	if (last_called_time == timer - 1) {
		printf("Received %u 32Byte Packet,%uKB/s\r\n", counter, counter * 32 / 1000);
		counter = 0;
	}
	last_called_time = timer;
}
void NRF24_Callback3(const uint8_t pipe, const uint8_t *pdata, const uint32_t len) {
	//printf("%s",pdata);
	uint8_t i = 0;
	//printf("RX len = %u \r\n",len);
	for (i = 0; i < len; i++) {
		if (pdata[i] == 0)
			break;
		else if( pdata[i] == '\r' || pdata[i]=='\n')
			putc(pdata[i], stdout);
		else if (pdata[i] >= 0x80 || (pdata[i]<0x20))
			putc('#', stdout);

		else
			putc(pdata[i], stdout);
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
	static uint8_t rx_p1_addr[] = { 1, 2, 3, 4, 5 };
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




	while ((result = getopt(argc, argv, "rS:C:")) != -1) {
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

#if 0
	NRF24_Write_Reg(DYPLD,0x3F);
	NRF24_Write_Reg(FEATURE, (1<<2)|(1<<0));
#else
	NRF24_Write_Reg(DYPLD, 0x00);
	NRF24_Write_Reg(FEATURE, 0x00);
#endif
	NRF24_Set_TX_Addr("812_0");
	NRF24_Set_RX_Addr(rx_p1_addr, 1);
	NRF24_SetRX_Length(0, 32);
	NRF24_SetRX_Length(1, 32);
	NRF24_Dump();
	NRF24_Listen(&nrf, 0, NRF24_Callback3);
}
