#pragma once
#include <cdefs.h>

#include "netutils.hpp"
#include "../mm/memory.hpp"

struct ARPPacketHeader_t {
#define ARP_HADRWARE_TYPE_ETH 0x1
	u16 HardwareType; // Hardware type
#define ARP_PROTOCOL_TYPE_IP 0x0800
	u16 ProtocolType; // Protocol type
#define ARP_PACKET_ETH_LENGTH 6
	u8  HardwareLen; // Hardware address length (Ethernet = 6)
#define ARP_PACKET_IPv4_LENGTH 4
	u8  ProtocolLen; // Protocol address length (IPv4 = 4)
#define ARP_REQUEST 0x0001
#define ARP_REPLY   0x0002
	u16 OPCode; // ARP Operation Code
}__attribute__((packed));

struct ARPPacketEthIPv4_t : public ARPPacketHeader_t {
	u8  SrcHardware[ARP_PACKET_ETH_LENGTH]; // Source hardware address - hlen bytes (see above)
	u8  SrcProtocol[ARP_PACKET_IPv4_LENGTH]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
	u8  DstHardware[ARP_PACKET_ETH_LENGTH]; // Destination hardware address - hlen bytes (see above)
	u8  DstProtocol[ARP_PACKET_IPv4_LENGTH]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
}__attribute__((packed));

int ARPSendPacket(NetworkCard *card, Heap *kernelHeap, u8 *dstHardwareAddr, u8 *dstProtocolAddr);
