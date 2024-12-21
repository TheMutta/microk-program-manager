#include "virtio-net.hpp"

#include "memory.hpp"
#include "capability.hpp"

#include <mkmi.h>

VirtIONetDevice_t *InitializeVirtIONetDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device) {
	VirtIONetDevice_t *netDevice = (VirtIONetDevice_t*)kernelHeap->Malloc(sizeof(VirtIONetDevice_t));
	netDevice->Device = device;
	
	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD;
	
	mkmi_log("Device features: 0x%x\r\n", device->Header->DeviceFeatures);
	device->Header->DeviceFeatureSelect = device->Header->DeviceFeatures;
	
	VirtIONetConfig_t *netConfig = (VirtIONetConfig_t*)((uptr)device->Header + device->ConfigOffset);
	mkmi_log("Max Virtqueue Pairs: %d\r\n", netConfig->MaxVirtqueuePairs);

	netDevice->VirtqueuePairs = netConfig->MaxVirtqueuePairs;
	netDevice->ControlVirtqueue = 2 * netConfig->MaxVirtqueuePairs;

	Capability utBufferCapability;

	Capability rxBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &rxBufferCapability, FRAME_MEMORY, 1);
	netDevice->RXBuffer = rxBufferCapability.Object;
	netDevice->RXBufferMapping = (u8*)mapper->MMap(&rxBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(netDevice->RXBufferMapping, 0, PAGE_SIZE);

	Capability txBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &txBufferCapability, FRAME_MEMORY, 1);
	netDevice->TXBuffer = txBufferCapability.Object;
	netDevice->TXBufferMapping = (u8*)mapper->MMap(&txBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(netDevice->TXBufferMapping, 0, PAGE_SIZE);

	Capability controlBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &controlBufferCapability, FRAME_MEMORY, 1);
	netDevice->ControlBuffer = controlBufferCapability.Object;
	netDevice->ControlBufferMapping = (u8*)mapper->MMap(&controlBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(netDevice->ControlBufferMapping, 0, PAGE_SIZE);

	return netDevice;
}
