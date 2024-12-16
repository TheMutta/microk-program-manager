#pragma once
#include <cdefs.h>

#include "virtio.hpp"

enum VirtIOGPUCtlType_t {
	/* 2d commands */
	VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
	VIRTIO_GPU_CMD_RESOURCE_UNREF,
	VIRTIO_GPU_CMD_SET_SCANOUT,
	VIRTIO_GPU_CMD_RESOURCE_FLUSH,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
	VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
	VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
	VIRTIO_GPU_CMD_GET_CAPSET_INFO,
	VIRTIO_GPU_CMD_GET_CAPSET,
	VIRTIO_GPU_CMD_GET_EDID,
	/* cursor commands */
	VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
	VIRTIO_GPU_CMD_MOVE_CURSOR,
	/* success responses */
	VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
	VIRTIO_GPU_RESP_OK_DISPLAY_INFO,
	VIRTIO_GPU_RESP_OK_CAPSET_INFO,
	VIRTIO_GPU_RESP_OK_CAPSET,
	VIRTIO_GPU_RESP_OK_EDID,
	/* error responses */
	VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
	VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
	VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};

#define VIRTIO_GPU_MAX_SCANOUTS 16
struct VirtIOGPURect_t {
	u32 X;
	u32 Y;
	u32 Width;
	u32 Height;
};

struct VirtIOGPUCtlHdr_t {
	u32 Type;
	u32 Flags;
	u64 FenceID;
	u32 CtxID;
	u32 Padding;
}__attribute__((packed));

struct VirtIOGPURespDisplayInfo_t {
	VirtIOGPUCtlHdr_t Header;
	struct VirtioGPUDisplayOne_t {
		VirtIOGPURect_t Rect;
		u32 Enabled;
		u32 Flags;
	} PModes[VIRTIO_GPU_MAX_SCANOUTS];
}__attribute__((packed));

enum VirtIOGPUFormats_t {
	VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM = 1,
	VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM = 2,
	VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM = 3,
	VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM = 4,
	VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM = 67,
	VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM = 68,
	VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM = 121,
	VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM = 134,
};

struct VirtIOGPUResourceCreate2D_t {
	VirtIOGPUCtlHdr_t Header;
	u32 ResourceID;
	u32 Format;
	u32 Width;
	u32 Height;
}__attribute__((packed));

struct VirtIOGPUResourceAttachBacking_t {
	VirtIOGPUCtlHdr_t Header;
	u32 ResourceID;
	u32 NrEntries;
}__attribute__((packed));

struct VirtIOGPUMemEntry {
	u64 Addr;
	u32 Length;
	u32 Padding;
}__attribute__((packed));

struct VirtIOGPUConfig_t {
	u32 EventsRead;
	u32 EventsClear;
	u32 NumScanouts;
	u32 Reserved0;
}__attribute__((packed));


struct VirtIOGPU_t {
	VirtIODevice_t *Device;

	volatile u8 *RequestBufferMapping;
	uptr RequestBuffer;

	volatile u8 *ResponseBufferMapping;
	uptr ResponseBuffer;
};

VirtIOGPU_t *InitializeVirtIOGPU(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device);
