#pragma once
#include <cdefs.h>
#include <object.hpp>

#include "memory.hpp"
#include "acpi.hpp"

#define DEVICE_ACK   1
#define DRIVER_LOAD  2
#define DRIVER_READY 4
#define DEVICE_ERROR 40
#define DRIVER_FAIL  80

enum VirtIODeviceTypes_t {
	VIRTIO_NET = 1,
	VIRTIO_BLOCK,
};

struct VirtIOHeader_t {
	u32 DeviceFeatureSelect;
	u32 DeviceFeatures;
	u32 GuestFeatureSelect;
	u32 GuestFeatures;
	u16 MSIXConfig;
	u16 NumQueues;
	u8 DeviceStatus;
	u8 ConfigGeneration;

	u16 QueueSelect;
	u16 QueueSize;
	u16 QueueMSIXVector;
	u16 QueueEnable;
	u16 QueueNotifyOff;
	u64 QueueDesc;
	u64 QueueAvailable;
	u64 QueueUsed;
}__attribute__((packed));

struct VirtIOQueueDesc_t {
	u64 Address;
	u32 Length;
#define VIRTQ_DESC_F_NEXT 1 
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4
	u16 Flags;
	u16 Next;
}__attribute__((packed));

struct VirtIOQueueAvail_t {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	u16 Flags;
	u16 Index;
	u16 Ring[];
	/* Only if VIRTIO_F_EVENT_IDX: u16 UsedEvent */
}__attribute__((packed));

struct VirtIOQueueUsedElement_t {
	u32 ID;
	u32 Length;
}__attribute__((packed));

struct VirtIOQueueUsed_t {
#define VIRTQ_USED_F_NO_NOTIFY 1
	u16 Flags;
	u16 Index;
	VirtIOQueueUsedElement_t Ring[];
	/* Only if VIRTIO_F_EVENT_IDX: u16 AvailEvent */
}__attribute__((packed));

struct VirtIOQueue_t {
	Capability *mmioMemoryArray;
	usize mmioMemoryCount;

	volatile VirtIOQueueDesc_t *Desc;
	volatile VirtIOQueueAvail_t *Avail;
	volatile VirtIOQueueUsed_t *Used;
};

struct VirtIODevice_t {
	Capability BARCapability;
	volatile VirtIOHeader_t *Header;
	usize ConfigOffset;

	usize QueueCount;
	VirtIOQueue_t *Queues;
};

VirtIODevice_t *InitializeVirtIODevice(Heap *kernelHeap, MemoryMapper *mapper, PCIHeader0_t *header0, PCICapability_t *pciCapabilityArray, usize pciCapabilityCount);


