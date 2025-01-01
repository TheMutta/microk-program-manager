#include "virtio.hpp"
#include "../init/capability.hpp"

#include <mkmi.h>

VirtIODevice_t *InitializeVirtIODevice(Heap *kernelHeap, MemoryMapper *mapper, PCIHeader0_t *header0) {
	VirtIODevice_t *device = (VirtIODevice_t*)kernelHeap->Malloc(sizeof(VirtIODevice_t));

	/* Enable busmaster */
	header0->Command |= 0b100;
	//__fast_hypercall(HYPERCALL_VECTOR_REGISTER_IRQ, header0->InterruptLine, 32, 0, 0, 0, 0);

	usize mainBar;
	usize cfgOffset;

	PCICapability_t *pciCapability = (PCICapability_t*)((uptr)header0 + header0->CapabilitiesPointer);
	usize pciCapabilityCount;
	for (pciCapabilityCount = 0;; pciCapabilityCount++) {
		mkmi_log("Capability:\r\n"
		  " ID: %x\r\n"
		  " Next: 0x%x\r\n"
		  " Length: 0x%x\r\n",
		  pciCapability->CapID,
		  pciCapability->CapNext,
		  pciCapability->CapLength);

		if (pciCapability->CapID == 0x9) {
			VirtIOPCICapability_t *current = (VirtIOPCICapability_t*)pciCapability;

			switch(current->CfgType) {
				case VIRTIO_PCI_CAP_COMMON_CFG:
					mkmi_log("Main bar is at: %x\r\n", current->BAR);
					mainBar = current->BAR;
					break;
				case VIRTIO_PCI_CAP_DEVICE_CFG:
					mkmi_log("Cfg bar is at: %x\r\n", current->BAR);
					mkmi_log("Cfg offset is at: %x\r\n", current->Offset);
					device->ConfigOffset = current->Offset;
					break;
			}
		} else if (pciCapability->CapID == 0x05) {
			mkmi_log("MSI\r\n");
		} else if (pciCapability->CapID == 0x11) {
			mkmi_log("MSI-X\r\n");

		}

		if (pciCapability->CapNext == 0) {
			pciCapabilityCount++;
			break;
		}

		pciCapability = (PCICapability_t*)((uptr)header0 + pciCapability->CapNext);
	}

	uptr barAddr;
	usize barSize;
	if (mainBar < 5) {
		barAddr = GetBAR(&header0->BAR[mainBar], &header0->BAR[mainBar + 1], &barSize);
	} else {
		barAddr = GetBAR(&header0->BAR[mainBar], 0, &barSize);
	}

	usize count = barSize / PAGE_SIZE;
	device->BARCapability = (Capability*)kernelHeap->Malloc(sizeof(Capability)*count);

	for (usize i = 0; i < count; ++i) {
		AddressCapability(barAddr + i * PAGE_SIZE, &device->BARCapability[i]);
	}

	device->Header = (volatile VirtIOHeader_t*)mapper->MMap(device->BARCapability, count, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	device->Header->DeviceStatus = DEVICE_ACK;
	mkmi_log("Device has %d queues\r\n", device->Header->NumQueues);

	device->QueueCount = device->Header->NumQueues;
	device->Queues = (VirtIOQueue_t*)kernelHeap->Malloc(sizeof(VirtIOQueue_t) * device->QueueCount);

	for (usize i = 0; i < device->QueueCount; ++i) {
		device->Header->QueueSelect = i;

		device->Queues[i].NextBuffer = 0;
		u32 queueSize = device->Header->QueueSize;
		device->Queues[i].QueueSize = queueSize;
		u32 sizeofBuffers = 16 * queueSize;
		u32 sizeofQueueAvailable = 6 + 2 * queueSize*sizeof(u16);
		u32 sizeofQueueUsed = 6 + (8 * queueSize);
		ROUND_UP_TO(sizeofBuffers, PAGE_SIZE);
		ROUND_UP_TO(sizeofQueueAvailable, PAGE_SIZE);
		ROUND_UP_TO(sizeofQueueUsed, PAGE_SIZE);
		u32 sizeTotal = sizeofBuffers + sizeofQueueAvailable + sizeofQueueUsed;
		ROUND_UP_TO(sizeTotal, PAGE_SIZE);

		Capability utMemory;
		GetUntypedRegion(sizeTotal, &utMemory);
		usize pageCount = sizeTotal / PAGE_SIZE;
		Capability mmioMemory[pageCount];

		mkmi_log("Page count: %d\r\n", pageCount);
		RetypeCapability(utMemory, mmioMemory, MMIO_MEMORY, pageCount);

		mkmi_log("Memory region for queue: 0x%x %d\r\n", mmioMemory[0].Object, sizeTotal);

		// TODO: fix these caps
		uptr addr = mmioMemory[0].Object;
		device->Header->QueueDesc = addr;
		device->Queues[i].DescPhys = device->Header->QueueDesc;
		device->Queues[i].Desc = (volatile VirtIOQueueDesc_t*)mapper->MMap(&mmioMemory[0], 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		device->Header->QueueAvailable = addr + sizeofBuffers;
		device->Queues[i].AvailPhys = device->Header->QueueAvailable;
		device->Queues[i].Avail = (volatile VirtIOQueueAvail_t*)mapper->MMap(&mmioMemory[1], 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		device->Header->QueueUsed = (addr + sizeofBuffers + sizeofQueueAvailable);
		device->Queues[i].UsedPhys = device->Header->QueueUsed;
		device->Queues[i].Used = (volatile VirtIOQueueUsed_t*)mapper->MMap(&mmioMemory[2], 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		device->Queues[i].Used->Flags = 0;

		u64 desc = device->Header->QueueDesc;
		mkmi_log("Desc: 0x%x\r\n", desc);
		u64 available = device->Header->QueueAvailable;
		mkmi_log("Driver: 0x%x\r\n", available);
		u64 used = device->Header->QueueUsed;
		mkmi_log("Device: 0x%x\r\n", used);

		device->Header->QueueEnable = 1;
	}

	return device;
}

void VirtIOSendBuffer(VirtIODevice_t *device, u16 queueIndex, VirtIOBufferInfo_t *bufferInfo, u64 count) {
	volatile VirtIOQueue_t *queue = &device->Queues[queueIndex];

	u16 index = queue->Avail->Index % queue->QueueSize;
	u16 bufferIndex = queue->NextBuffer;
	u16 nextBufferIndex;

	unsigned char *buf = (u8*)(&queue->Desc[sizeof(VirtIOQueueDesc_t)*bufferIndex]);
	unsigned char *buf2 = buf;

	queue->Avail->Ring[index] = bufferIndex;
	for (usize i = 0; i < count; i++) {
		nextBufferIndex = (bufferIndex+1) % queue->QueueSize;

		VirtIOBufferInfo_t *bi = &bufferInfo[i];
		queue->Desc[bufferIndex].Flags = bi->Flags;
		if (i != (count-1)) {
			queue->Desc[bufferIndex].Flags |= VIRTQ_DESC_F_NEXT;
		}

		queue->Desc[bufferIndex].Next = nextBufferIndex;
		queue->Desc[bufferIndex].Length = bi->Size;

		if (bi->Copy && false) {
			// TODO: fix
			queue->Desc[bufferIndex].Address = (uptr)((uptr)buf2 - (uptr)queue->Desc + queue->DescPhys);
			if (bi->Buffer != NULL) {
				memcpy(buf2, bi->Buffer, bi->Size);
			}
			buf2 += bi->Size;
		} else {
			// calling function wants to keep same buffer
			queue->Desc[bufferIndex].Address = bi->Buffer;
		}
		bufferIndex = nextBufferIndex;
	}

	queue->NextBuffer = bufferIndex;

	queue->Avail->Index++;
}
