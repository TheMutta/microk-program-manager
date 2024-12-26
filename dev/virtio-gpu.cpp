#include "virtio-gpu.hpp"

#include "../init/capability.hpp"

#include <mkmi.h>

VirtIOGPU_t *InitializeVirtIOGPU(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device) {
	VirtIOGPU_t *gpu = (VirtIOGPU_t*)kernelHeap->Malloc(sizeof(VirtIOGPU_t));
	gpu->Device = device;

	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD;

	Capability utBufferCapability;
	Capability requestBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &requestBufferCapability, FRAME_MEMORY, 1);
	gpu->RequestBuffer = requestBufferCapability.Object;
	gpu->RequestBufferMapping = (u8*)mapper->MMap(&requestBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(gpu->RequestBufferMapping, 0, PAGE_SIZE);

	Capability responseBufferCapability;
	GetUntypedRegion(PAGE_SIZE, &utBufferCapability);
	RetypeCapability(utBufferCapability, &responseBufferCapability, FRAME_MEMORY, 1);
	gpu->ResponseBuffer = responseBufferCapability.Object;
	gpu->ResponseBufferMapping = (u8*)mapper->MMap(&responseBufferCapability, 1, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	memset(gpu->ResponseBufferMapping, 0, PAGE_SIZE);

	mkmi_log("Device features: 0x%x\r\n", device->Header->DeviceFeatures);
	//device->Header->DeviceFeatureSelect = device->Header->DeviceFeatures;

	VirtIOGPUConfig_t *config = (VirtIOGPUConfig_t*)((uptr)device + device->ConfigOffset);
	mkmi_log("Events Read: %d\r\n", config->EventsRead);
	mkmi_log("Events Clear: %d\r\n", config->EventsClear);
	mkmi_log("Num scanouts: %d\r\n", config->NumScanouts);

	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD | DRIVER_READY;

	// TODO: ????
	volatile VirtIOGPUCtlHdr_t *request = (volatile VirtIOGPUCtlHdr_t*)gpu->RequestBufferMapping;
	request->Type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
	request->Flags = 0;
	request->FenceID = 0;
	request->CtxID = 0;
	
	VirtIOBufferInfo_t bufferInfo[2];
	bufferInfo[0].Buffer = gpu->RequestBuffer;
	bufferInfo[0].Size = sizeof(VirtIOGPUCtlHdr_t);
	bufferInfo[0].Flags = 0;
	bufferInfo[1].Buffer = gpu->ResponseBuffer;
	bufferInfo[1].Size = sizeof(VirtIOGPURespDisplayInfo_t);
	bufferInfo[1].Flags = VIRTQ_DESC_F_WRITE;

	VirtIOSendBuffer(gpu->Device, 0, bufferInfo, 2);

	volatile VirtIOGPURespDisplayInfo_t *info = (volatile VirtIOGPURespDisplayInfo_t*)gpu->ResponseBufferMapping;
	//while(info->Header.Type != VIRTIO_GPU_RESP_OK_DISPLAY_INFO) { }




	return gpu;
}
