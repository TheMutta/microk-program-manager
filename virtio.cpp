#include "virtio.hpp"
#include "capability.hpp"

#include <mkmi.h>

VirtIODevice_t *InitializeVirtIODevice(Heap *kernelHeap, MemoryMapper *mapper, PCIHeader0_t *header0, PCICapability_t *pciCapabilityArray, usize pciCapabilityCount) {
	VirtIODevice_t *device = (VirtIODevice_t*)kernelHeap->Malloc(sizeof(VirtIODevice_t));

	usize mainBar;
	usize cfgOffset;

	for (usize i = 0; i < pciCapabilityCount; ++i) {
		/*mkmi_log("Capability:\r\n"
		  " ID: %x\r\n"
		  " Next: 0x%x\r\n"
		  " Length: 0x%x\r\n"
		  " CFG Type: 0x%x\r\n"
		  " BAR: 0x%x\r\n"
		  " Offset: 0x%x\r\n"
		  " Length: 0x%x\r\n", 
		  pciCapabilityArray[i].CapID,
		  pciCapabilityArray[i].CapNext,
		  pciCapabilityArray[i].CapLength,
		  pciCapabilityArray[i].CfgType,
		  pciCapabilityArray[i].BAR,
		  pciCapabilityArray[i].Offset,
		  pciCapabilityArray[i].Length
		  );*/

		if (pciCapabilityArray[i].CapID == 0x9) {
			if (pciCapabilityArray[i].CfgType == 1) {
				mkmi_log("Main bar is at: %x\r\n", pciCapabilityArray[i].BAR);
				mainBar = pciCapabilityArray[i].BAR;
			} else if (pciCapabilityArray[i].CfgType == 4) {
				mkmi_log("Cfg bar is at: %x\r\n", pciCapabilityArray[i].BAR);
				mkmi_log("Cfg offset is at: %x\r\n", pciCapabilityArray[i].Offset);
				device->ConfigOffset = pciCapabilityArray[i].Offset;
			}
		}
	}

	uptr barAddr;
	if (mainBar < 5) {
		barAddr = GetBAR(header0->BAR[mainBar], header0->BAR[mainBar + 1]);
	} else {
		barAddr = GetBAR(header0->BAR[mainBar], 0);
	}

	AddressCapability(barAddr, &device->BARCapability);
	device->Header = (volatile VirtIOHeader_t*)mapper->MMap(device->BARCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	device->Header->DeviceStatus = DEVICE_ACK;
	mkmi_log("Device has %d queues\r\n", device->Header->NumQueues);

	device->QueueCount = device->Header->NumQueues;
	device->Queues = (VirtIOQueue_t*)kernelHeap->Malloc(sizeof(VirtIOQueue_t));

	for (usize i = 0; i < device->QueueCount; ++i) {
		device->Header->QueueSelect = i;

		u32 queueSize = device->Header->QueueSize;
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
		device->Queues[i].Desc = (volatile VirtIOQueueDesc_t*)mapper->MMap(mmioMemory[0], PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		device->Header->QueueAvailable = addr + sizeofBuffers;
		device->Queues[i].Avail = (volatile VirtIOQueueAvail_t*)mapper->MMap(mmioMemory[1], PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		device->Header->QueueUsed = (addr + sizeofBuffers + sizeofQueueAvailable);
		device->Queues[i].Used = (volatile VirtIOQueueUsed_t*)mapper->MMap(mmioMemory[2], PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

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

/*void SendBuffer(VirtIODevice_t *device, u16 queueIndex, VirtIOBufferInfo_t, u64 count) {
	VirtIOQueue_t *queue = &device->Queues[queueIndex];
}*/
