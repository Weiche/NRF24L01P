#ifndef NRF24L01_NET_H
#define NRF24L01_NET_H

#include <stdint.h>
#include <string.h>
#include "nrf24l01.h"
typedef enum{
	type_noack_bit = 0x01,
	type_arp = 0x02,
	type_normal = 0x04
}packet_type_t;

typedef struct{
	uint8_t ip;
}nrf24_node_t;

typedef __packed struct {
	uint8_t type;
	uint8_t dst_ip;
	uint8_t src_ip;
	uint8_t payload[29];
}network_packet_t;

typedef struct{
	NRF24_InitTypedef* ifnet;
	nrf24_node_t *node_table;
	uint8_t network_addr[4];
	uint8_t self_ip;
}network_info_t;

extern network_info_t Network;

int NRF24_Net_Set_RXAddr( const uint8_t rx_addr, uint8_t pipe );

int NRF24_Net_Init( NRF24_InitTypedef* ifnet, const uint8_t* network_mac, const uint8_t self_ip);

int NRF24_Net_Sent( const uint8_t dst_ip, uint8_t* packet, const uint8_t len );

__weak void NRF24_Net_ReceiveCallback(const uint8_t* payload , const uint32_t len);

void NRF24_Net_ReceiveStart( void );

void NRF24_Net_ReceiveTask( void );

int NRF24_Net_ARP_Sent( const uint8_t dst_ip );



#endif
