#include "virtio-disk.hpp"

#include <mkmi.h>

VirtIOBlockDevice_t *InitializeVirtIOBlockDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device) {
	// TODO: heap bug
	VirtIOBlockDevice_t *blockDevice = (VirtIOBlockDevice_t*)kernelHeap->Malloc(sizeof(VirtIOBlockDevice_t));
	blockDevice->Device = device;
									
	device->Header->DeviceStatus = DEVICE_ACK | DRIVER_LOAD;

	VirtIOBlockConfig_t *blockConfig = (VirtIOBlockConfig_t*)((uptr)device->Header + device->ConfigOffset);
	mkmi_log("Capacity: %d sectors\r\n", blockConfig->Capacity);
	mkmi_log("Sector size: %d bytes\r\n", blockConfig->BlockSize);

	return blockDevice;
}
