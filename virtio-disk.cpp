#include "virtio-disk.hpp"
#include "memory.hpp"
#include "capability.hpp"

#include "fat.hpp"
#include "gpt.hpp"

#include <mkmi.h>

VirtIOBlockDevice_t *InitializeVirtIOBlockDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device) {
	VirtIOBlockDevice_t *blockDevice = (VirtIOBlockDevice_t*)kernelHeap->Malloc(sizeof(VirtIOBlockDevice_t));
	blockDevice->Device = device;
									
	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD;

	Capability utBufferCapability;
	Capability dmaBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &dmaBufferCapability, MMIO_MEMORY, 1);

	blockDevice->DiskBuffer = dmaBufferCapability.Object;
	blockDevice->DiskBufferMapping = (u8*)mapper->MMap(&dmaBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(blockDevice->DiskBufferMapping, 0, PAGE_SIZE);


	Capability requestBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &requestBufferCapability, FRAME_MEMORY, 1);
	blockDevice->RequestBuffer = requestBufferCapability.Object;
	blockDevice->RequestBufferMapping = (u8*)mapper->MMap(&requestBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(blockDevice->RequestBufferMapping, 0, PAGE_SIZE);

	
	mkmi_log("Device features: 0x%x\r\n", device->Header->DeviceFeatures);

	device->Header->DeviceFeatureSelect = device->Header->DeviceFeatures;

	VirtIOBlockConfig_t *blockConfig = (VirtIOBlockConfig_t*)((uptr)device->Header + device->ConfigOffset);
	mkmi_log("Capacity: %d sectors\r\n", blockConfig->Capacity);
	
	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD | DRIVER_READY;
/*
	VirtIOBlockRead(blockDevice, 0);
	FATHeader_t fatHeader = *(FATHeader_t*)blockDevice->DiskBufferMapping;
	mkmi_log("Boot Indicator: 0x%x 0x%x 0x%x\r\n", 
		fatHeader.Reserved0[0],
		fatHeader.Reserved0[1],
		fatHeader.Reserved0[2]
	);*/
	
	VirtIOBlockRead(blockDevice, 1);
	GPTHeader_t *header = (GPTHeader_t*)blockDevice->DiskBufferMapping;
	mkmi_log("Signature: %s\r\n", header->Signature);
	mkmi_log("GUID: %x-%x-%x-%x\r\n", 
			header->GUID.Groups.Data1, 
			header->GUID.Groups.Data2,
			header->GUID.Groups.Data3,
			header->GUID.Groups.Data4
			);

	mkmi_log("Partition entries: %d\r\n", header->PartitionEntries);
	
	/*
	mkmi_log("First sector: %s\r\n", fatHeader.OEMID);
	mkmi_log("Bytes per sector: %d\r\n", fatHeader.BytesPerSector);
	mkmi_log("FSInfo sector: %d\r\n", fatHeader.FSInfoSector);
	VirtIOBlockRead(blockDevice, fatHeader.FSInfoSector);
	FAT32FSInfo_t fsInfo = *(FAT32FSInfo_t*)blockDevice->DiskBufferMapping;
	mkmi_log("FSInfo signature: %s\r\n", fsInfo.Signature);*/

	


	return blockDevice;
}

void VirtIOBlockRead(VirtIOBlockDevice_t *blockDevice, usize sector) {
	VirtIOBufferInfo_t bufferInfo[3];
	VirtIOBlockRequestHeader_t *blockRequest = (VirtIOBlockRequestHeader_t*)blockDevice->RequestBufferMapping;
	u8 *status = (u8*)((uptr)blockDevice->RequestBuffer + PAGE_SIZE / 2);
	blockRequest->Type = VIRTIO_BLK_T_IN;
	blockRequest->Reserved = 0;
	blockRequest->Sector = sector;
	bufferInfo[0].Buffer = blockDevice->RequestBuffer;
	bufferInfo[0].Size = sizeof(VirtIOBlockRequestHeader_t);
	bufferInfo[0].Flags = 0;
	bufferInfo[1].Buffer = blockDevice->DiskBuffer;
	bufferInfo[1].Size = 512;
	bufferInfo[1].Flags = VIRTQ_DESC_F_WRITE;
	bufferInfo[2].Buffer = (uptr)status;
	bufferInfo[2].Size = 1;
	bufferInfo[2].Flags = VIRTQ_DESC_F_WRITE;

	VirtIOSendBuffer(blockDevice->Device, 0, bufferInfo, 3);
}

void VirtIOBlockWrite(VirtIOBlockDevice_t *blockDevice, usize sector) {
	VirtIOBufferInfo_t bufferInfo[3];
	VirtIOBlockRequestHeader_t *blockRequest = (VirtIOBlockRequestHeader_t*)blockDevice->RequestBufferMapping;
	u8 *status = (u8*)((uptr)blockDevice->RequestBuffer + PAGE_SIZE / 2);
	blockRequest->Type = VIRTIO_BLK_T_OUT;
	blockRequest->Reserved = 0;
	blockRequest->Sector = sector;
	bufferInfo[0].Buffer = blockDevice->RequestBuffer;
	bufferInfo[0].Size = sizeof(VirtIOBlockRequestHeader_t);
	bufferInfo[0].Flags = 0;
	bufferInfo[1].Buffer = blockDevice->DiskBuffer;
	bufferInfo[1].Size = 512;
	bufferInfo[1].Flags = 0;
	bufferInfo[2].Buffer = (uptr)status;
	bufferInfo[2].Size = 1;
	bufferInfo[2].Flags = VIRTQ_DESC_F_WRITE;

	VirtIOSendBuffer(blockDevice->Device, 0, bufferInfo, 3);
}
