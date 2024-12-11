#include "virtio-disk.hpp"
#include "memory.hpp"
#include "capability.hpp"

#include <mkmi.h>

VirtIOBlockDevice_t *InitializeVirtIOBlockDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device) {
	// TODO: heap bug
	VirtIOBlockDevice_t *blockDevice = (VirtIOBlockDevice_t*)kernelHeap->Malloc(sizeof(VirtIOBlockDevice_t));
	blockDevice->Device = device;
									
	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD;

	Capability utBufferCapability;
	Capability dmaBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &dmaBufferCapability, MMIO_MEMORY, 1);

	blockDevice->DiskBuffer = dmaBufferCapability.Object;
	blockDevice->DiskBufferMapping = (u8*)mapper->MMap(dmaBufferCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(blockDevice->DiskBufferMapping, 0, PAGE_SIZE);


	Capability requestBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &requestBufferCapability, FRAME_MEMORY, 1);
	blockDevice->RequestBuffer = requestBufferCapability.Object;
	blockDevice->RequestBufferMapping = (u8*)mapper->MMap(requestBufferCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(blockDevice->RequestBufferMapping, 0, PAGE_SIZE);


	
	mkmi_log("Device features: 0x%x\r\n", device->Header->DeviceFeatures);

	VirtIOBlockConfig_t *blockConfig = (VirtIOBlockConfig_t*)((uptr)device->Header + device->ConfigOffset);
	mkmi_log("Capacity: %d sectors\r\n", blockConfig->Capacity);
	mkmi_log("Sector size: %d bytes\r\n", blockConfig->BlockSize);
	
	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD | DRIVER_READY;

	VirtIOBufferInfo_t bufferInfo[3];
	VirtIOBlockRequestHeader_t *blockRequest = (VirtIOBlockRequestHeader_t*)blockDevice->RequestBufferMapping;
	char *c = (char*)((uptr)blockDevice->RequestBuffer + PAGE_SIZE / 2);
	blockRequest->Type = VIRTIO_BLK_T_IN;
	blockRequest->Reserved = 0;
	blockRequest->Sector = 0;
	bufferInfo[0].Buffer = blockDevice->RequestBuffer;
	bufferInfo[0].Size = sizeof(VirtIOBlockRequestHeader_t);
	bufferInfo[0].Flags = 0;
	bufferInfo[0].Copy = false;
	bufferInfo[1].Buffer = blockDevice->DiskBuffer;
	bufferInfo[1].Size = 512;
	bufferInfo[1].Flags = VIRTQ_DESC_F_WRITE;
	bufferInfo[1].Copy = false;
	bufferInfo[2].Buffer = (uptr)c;
	bufferInfo[2].Size = 1;
	bufferInfo[2].Flags = VIRTQ_DESC_F_WRITE;
	bufferInfo[2].Copy = false;

	VirtIOSendBuffer(device, 0, bufferInfo, 3);

	while (true) {
		if(blockDevice->DiskBufferMapping[0] != 0) {
			mkmi_log("First sector: %s\r\n", blockDevice->DiskBufferMapping);
			break;
		}
	}

	return blockDevice;
}
