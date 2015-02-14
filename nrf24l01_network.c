
#include "nrf24l01.h"
#include <string.h>
/* imcomplete */
/* pipe assignment */
/*
example network mac 2345
Pipe0 for ack				FF 02 03 04 05
Pipe1 for self				0A 02 03 04 05

Pipe2 for broadcast(noack)	FF
Pipe3 for node2				0C
Pipe4 for node2				0D
Pipe5 for node				FF

*/
/*
Packet
--Type 1byte
--Payload 1-31bytes
*/

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
/**/
network_info_t Network = {
	.network_addr={
		2,3,4,5
	},
	.node_table = NULL
};

int NRF24_Net_Set_RXAddr( const uint8_t rx_addr, uint8_t pipe ){
	uint8_t addr[5];
	
	memcpy( addr + 1, (uint8_t*)Network.network_addr, 4);
	addr[0] = rx_addr;
	NRF24_Set_RX_Addr( addr, pipe );
	return 0;
}

int NRF24_Net_Init( NRF24_InitTypedef* ifnet, const uint8_t* network_mac, const uint8_t self_ip){
	Network.ifnet = ifnet;
	Network.self_ip = self_ip;
	
	Network.network_addr[0] = network_mac[0];
	Network.network_addr[1] = network_mac[1];
	Network.network_addr[2] = network_mac[2];
	Network.network_addr[3] = network_mac[3];
	
	NRF24_Net_Set_RXAddr(self_ip,1);
	//NRF24_Net_Set_RXAddr(0xFF,2);
	
	NRF24_SetRX_Length(0, 32);
	NRF24_SetRX_Length(1, 32);
	/*
	NRF24_SetRX_Length(2, 32);
	NRF24_SetRX_Length(3, 32);
	NRF24_SetRX_Length(4, 32);
	NRF24_SetRX_Length(5, 32);
	*/
	return 0;
}

int NRF24_Net_Sent( const uint8_t dst_ip, uint8_t* packet, const uint8_t len ){
	uint8_t addr[5];
	uint8_t noack_flag = (packet[0]&0x01);
	
	/* find the mac for dst */
	
	/* write dst_ip and src_ip*/
	packet[1] = dst_ip;
	packet[2] = Network.self_ip;
	
	/* set TX address */
	addr[0] = dst_ip;
	addr[1] = Network.network_addr[0];
	addr[2] = Network.network_addr[1];
	addr[3] = Network.network_addr[2];
	addr[4] = Network.network_addr[3];
	
	/* clock into TX fifo */
	if( dst_ip != 255 ){		
		return NRF24_SendPacketBlocking(addr, packet, len, noack_flag );
	}else{
		return NRF24_SendPacketBlocking(addr, packet, len, 1 );
	}
	/* wait for transimission */
}
__weak void NRF24_Net_ReceiveCallback(const uint8_t* payload , const uint32_t len){
	__nop();
}



void NRF24_Net_ReceiveTask( void ){
	static uint8_t data[32],status,pipe;
	static uint32_t length;
	
	status = NRF24_Read_Reg(STATUS);
	if( status & STATUS_RX_DR ){
		if( NRF24_GetReceivedPacket( data, &length, &pipe , status) == 0 ){
			/* if not a packet for self or not a router or not a broadcast */
			if( (data[1] != Network.self_ip) && (data[1] != 0xFF) && (Network.self_ip != 1) ){
				return;
			}else{
				NRF24_Net_ReceiveCallback( data , length);
			}
		}
	}
}



int NRF24_Net_ARP_Sent( const uint8_t dst_ip ){
	uint8_t packet[32];
	packet[0] = type_arp;	
	
	return NRF24_Net_Sent( dst_ip, packet, 32 );
}


