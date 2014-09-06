#ifdef NRF24_HAL_LINUX
/******************************************
 *Don`t include nrf24l01.h in this file
 *******************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../nrf24l01_hal.h"

#define NRF24_SPIDEV "/dev/spidev0.0"
#define NRF24_CE_PIN 17

int mode = 0, bits = 8, speed = 10000000, delay = 1;
int nrf24_spi;
int CE_Pin_Dir, CE_Pin_Value;
uint8_t trash_rx[128];
uint8_t dummy_tx[128];

static void pabort(char *str) {

	printf("%s\r\n", str);
	exit(1);
}
static void transfer(const uint8_t *tx_buffer, uint8_t *rx_buffer,
		uint32_t length) {
	int ret;
	memset(rx_buffer, 0, length);
	struct spi_ioc_transfer tr = { .tx_buf = (unsigned long) tx_buffer,
			.rx_buf = (unsigned long) rx_buffer, .len = length, .delay_usecs =
					delay, .speed_hz = speed, .bits_per_word = bits, };

	ret = ioctl(nrf24_spi, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

}

static void NRF24_CE_Init(void) {
	char ce_pinpath[64];
	char temp[64];
	int file;
	/* Export GPIO */
	file = open("/sys/class/gpio/export", O_RDWR);
	if( file < 0 ){
		pabort("Can not export GPIO for CE");
	}
	write(file, temp, sprintf(temp, "%d", NRF24_CE_PIN));
	close(file);

	sprintf(ce_pinpath, "/sys/class/gpio/gpio%d/", NRF24_CE_PIN);
	/* Set Direction of CE Pin */
	sprintf(temp, "%sdirection", ce_pinpath);
	printf("Open %s !\r\n", temp);
	if ((CE_Pin_Dir = open(temp, O_RDWR)) < 0) {
		/* ERROR HANDLING: you can check error to see what went wrong */
		pabort("Failed to open the pin ce direction\r\n");

	}

	write(CE_Pin_Dir, "out", 3);
	/* Reset CE to low */
	sprintf(temp, "%svalue", ce_pinpath);
	printf("Open %s !\r\n", temp);
	if ((CE_Pin_Value = open(temp, O_RDWR)) < 0) {
		/* ERROR HANDLING: you can check error to see what went wrong */
		pabort("Failed to open the pin ce value\r\n");

	}
	write(CE_Pin_Value, "0", 1);

}
int msleep( int ms ){
	return usleep(ms * 1000);
}

int NRF24_HAL_Init( void ) {

	char *filename = NRF24_SPIDEV;

	int ret;
	int file;

	NRF24_OS_Functions.Printf = printf;
	NRF24_OS_Functions.DelayMs = msleep;
	/* Init CE */
	NRF24_CE_Init();
	/* fill the dummy_tx with 0xFF */
	memset(dummy_tx, 0xFF, 128);
	/* Init SPI mode 0 should be set*/
	printf("Open %s !\r\n", filename);
	if ((nrf24_spi = open(filename, O_RDWR)) < 0) {
		/* ERROR HANDLING: you can check error to see what went wrong */
		printf("Failed to open the spi\r\n");
		exit(1);
	}
	/*
	 * spi mode
	 */
	ret = ioctl(nrf24_spi, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(nrf24_spi, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(nrf24_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(nrf24_spi, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(nrf24_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(nrf24_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);
	return 0;
}
uint8_t NRF24_SendReceiveByte(uint8_t data) {
	uint8_t rx_buffer[1];
	transfer(&data, rx_buffer, 1);
	return rx_buffer[0];
}

void NRF24_Enable(void) {
	return;
}
void NRF24_Disable(void) {
	return;
}

void NRF24_CE_Enable(void) {
	write(CE_Pin_Value, "1", 1);
}
void NRF24_CE_Disable(void) {
	write(CE_Pin_Value, "0", 1);
}

void NRF24_InsWrite(uint8_t instruction, const uint8_t *pbuff, uint32_t num) {
	static uint8_t temp[256];
	temp[0] = instruction;
	memcpy(temp + 1, pbuff, num);
	transfer(temp, trash_rx, num + 1);
}
void NRF24_InsRead(uint8_t instruction, uint8_t *pbuff, uint32_t num) {
	static uint8_t temp[256];
	dummy_tx[0] = instruction;
	transfer(dummy_tx, temp, num + 1);
	dummy_tx[0] = 0xFF;
	memcpy(pbuff, &temp[1], num);
}

#endif

