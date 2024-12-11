#pragma once
#include "virtio.hpp"

struct VirtIOBlockConfig_t {
	u64 Capacity;
	u32 size_max;
	u32 seg_max;
	struct virtio_blk_geometry {
		u16 cylinders;
		u8 heads;
		u8 sectors;
	} geometry;
	u32 blk_size;
	struct virtio_blk_topology {
		// # of logical blocks per physical block (log2)
		u8 physical_block_exp;
		// offset of first aligned logical block
		u8 alignment_offset;
		// suggested minimum I/O size in blocks
		u16 min_io_size;
		// optimal (suggested maximum) I/O size in blocks
		u32 opt_io_size;
	} topology;
	u8 writeback;
	u8 unused0[3];
	u32 max_discard_sectors;
	u32 max_discard_seg;
	u32 discard_sector_alignment;
	u32 max_write_zeroes_sectors;
	u32 max_write_zeroes_seg;
	u8 write_zeroes_may_unmap;
	u8 unused1[3];
};

struct BlockRequestHeader_t { 
    u32 type; 
    u32 reserved; 
    u64 sector; 
};

struct VirtIOBlockDevice_t {
	VirtIODevice_t *Device;
};

VirtIOBlockDevice_t *InitializeVirtIOBlockDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device);
