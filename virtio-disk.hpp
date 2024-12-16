#pragma once
#include "virtio.hpp"

#define VIRTIO_BLK_F_SIZE_MAX 1
#define VIRTIO_BLK_F_SEG_MAX  2
#define VIRTIO_BLK_F_SEG_GEOMETRY 4

struct VirtIOBlockConfig_t {
	u64 Capacity;
	u32 SizeMax;
	u32 seg_max;
	struct virtio_blk_geometry {
		u16 cylinders;
		u8 heads;
		u8 sectors;
	} geometry;
	u32 BlockSize;
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
}__attribute__((packed));

struct VirtIOBlockRequestHeader_t { 
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define VIRTIO_BLK_T_FLUSH 4
#define VIRTIO_BLK_T_DISCARD 11
#define VIRTIO_BLK_T_WRITE_ZEROES 13
    u32 Type; 
    u32 Reserved; 
    u64 Sector; 
    u8 Data[];
    //u8 Status;
}__attribute__((packed));

struct VirtIOBlockDevice_t {
	VirtIODevice_t *Device;

	uptr DiskBuffer;
	u8* DiskBufferMapping;

	uptr RequestBuffer;
	u8* RequestBufferMapping;
};

VirtIOBlockDevice_t *InitializeVirtIOBlockDevice(Heap *kernelHeap, MemoryMapper *mapper, VirtIODevice_t *device);


void VirtIOBlockRead(VirtIOBlockDevice_t *blockDevice, usize sector);
void VirtIOBlockWrite(VirtIOBlockDevice_t *blockDevice, usize sector);
