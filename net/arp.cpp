#include "arp.hpp"
#include "ethernet.hpp"
#include "netutils.hpp"

#include <mkmi.h>

static u8 BroadcastMACAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int ARPSendPacket(NetworkCard *card, Heap *kernelHeap, u8 *dstHardwareAddr, u8 *dstProtocolAddr) {
	ARPPacketEthIPv4_t packet;

	packet.HardwareType = htons(ARP_HADRWARE_TYPE_ETH);
	packet.ProtocolType = htons(ARP_PROTOCOL_TYPE_IP);

	packet.HardwareLen = ARP_PACKET_ETH_LENGTH;
	packet.ProtocolLen = ARP_PACKET_IPv4_LENGTH;

	packet.OPCode = htons(ARP_REQUEST);

	memcpy(packet.SrcHardware, card->MAC, ARP_PACKET_ETH_LENGTH);
	packet.SrcProtocol[0] = 10;
	packet.SrcProtocol[1] = 0;
	packet.SrcProtocol[2] = 0;
	packet.SrcProtocol[3] = 10;

	memcpy(packet.DstHardware, dstHardwareAddr, ARP_PACKET_ETH_LENGTH);
	memcpy(packet.DstProtocol, dstProtocolAddr, ARP_PACKET_IPv4_LENGTH);
	
	return EthernetSendPacket(card, kernelHeap, BroadcastMACAddress, (u8*)&packet, sizeof(packet), ETH_TYPE_ARP);
}
