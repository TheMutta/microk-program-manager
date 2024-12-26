#pragma once
#include "virtio.hpp"

struct VirtIONetConfig_t {
	u8 MAC[6];
	u16 Status;
	u16 MaxVirtqueuePairs;
	u16 MTU;
}__attribute__((packed));

struct VirtIONetHdr_t { 
#define VIRTIO_NET_HDR_F_NEEDS_CSUM    1 
#define VIRTIO_NET_HDR_F_DATA_VALID    2 
#define VIRTIO_NET_HDR_F_RSC_INFO      4 
        u8 Flags; 
#define VIRTIO_NET_HDR_GSO_NONE        0 
#define VIRTIO_NET_HDR_GSO_TCPV4       1 
#define VIRTIO_NET_HDR_GSO_UDP         3 
#define VIRTIO_NET_HDR_GSO_TCPV6       4 
#define VIRTIO_NET_HDR_GSO_ECN      0x80 
        u8 GSOType; 
        u16 HdrLen; 
        u16 GSOSize; 
        u16 ChecksumStart; 
        u16 ChecksumOffset;
        u16 NumBuffers; 
};

struct VirtIONetDevice_t {
	VirtIODevice_t *Device;

	usize VirtqueuePairs;
	usize ControlVirtqueue;

	uptr RXBuffer;
	u8* RXBufferMapping;

	uptr TXBuffer;
	u8* TXBufferMapping;

	uptr ControlBuffer;
	u8* ControlBufferMapping;


};

VirtIONetDevice_t *InitializeVirtIONetDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device);
