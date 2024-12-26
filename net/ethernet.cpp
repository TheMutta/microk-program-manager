#include "ethernet.hpp"
#include <mkmi.h>

int EthernetSendPacket(NetworkCard *card, Heap *kernelHeap, u8 *dstMacAddr, u8 *data, usize len, u16 protocol) {
	EthernetFrame_t *frame = (EthernetFrame_t*)kernelHeap->Malloc(sizeof(EthernetFrame_t) + len);

	memcpy(frame->DstMacAddr, dstMacAddr, ETH_MAC_LENGTH);
	memcpy(frame->SrcMacAddr, card->MAC, ETH_MAC_LENGTH);

	frame->Type = htons(protocol);

	memcpy(frame->Data, data, len);

	card->SendPacket(card, (u8*)frame, sizeof(EthernetFrame_t) + len);

	return 0;
}
