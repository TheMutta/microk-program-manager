#pragma once
#include <cdefs.h>

#include "netutils.hpp"
#include "../mm/memory.hpp"

struct EthernetFrame_t {
#define ETH_MAC_LENGTH 6
	u8 DstMacAddr[ETH_MAC_LENGTH];
	u8 SrcMacAddr[ETH_MAC_LENGTH];
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IP 0x0800
	u16 Type;
	u8 Data[];
}__attribute__((packed));

int EthernetSendPacket(NetworkCard *card, Heap *kernelHeap, u8 *dstMacAddr, u8 *data, usize len, u16 protocol);
