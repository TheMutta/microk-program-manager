#pragma once
#include "virtio.hpp"

struct VirtIONetConfig_t {
	u8 MAC[6];
	u16 Status;
	u16 MaxVirtqueuePairs;
	u16 MTU;
}__attribute__((packed));

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
