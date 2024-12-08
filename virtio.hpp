#pragma once
#include <cdefs.h>

#define DEVICE_ACK   1
#define DRIVER_LOAD  2
#define DRIVER_READY 4
#define DEVICE_ERROR 40
#define DRIVER_FAIL  80

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
	u64 QueueAvail;
	u64 QueueUsed;
}__attribute__((packed));

struct VirtIOQueueBuffer_t {
	u64 Address;
	u32 Length;
	u16 Flags;
	u16 Next;
}__attribute__((packed));

struct VirtIOUsedItem_t {
    u32 Index;
    u32 Length;
}__attribute__((packed));

struct VirtIOAvailableItem_t {
    u32 Index;
    u64 dress;
    u32 length;
}__attribute__((packed));


struct VirtIONetHeader_t {
	u8 MacAddress[6];
	u16 Status;
	u16 MaxVirtqueuePairsS;
	u16 MTU;
	u32 Speed;
	u8 Duplex;
	u8 RSSMaxKeySize;
	u16 RSSMaxIndirectionTableLen;
	u32 SupportedHashTypes;
}__attribute__((packed));


struct VirtIOBlockHeader_t {
	/* The capacity (in 512-byte sectors). */
	u64 Capacity;
	/* The maximum segment size (if VIRTIO_BLK_F_SIZE_MAX) */
	u32 SizeMax;
	/* The maximum number of segments (if VIRTIO_BLK_F_SEG_MAX) */
	u32 seg_max;
	/* geometry of the device (if VIRTIO_BLK_F_GEOMETRY) */
	struct virtio_blk_geometry {
		u16 cylinders;
		u8 heads;
		u8 sectors;
	} geometry;

	/* block size of device (if VIRTIO_BLK_F_BLK_SIZE) */
	u32 blk_size;

	/* the next 4 entries are guarded by VIRTIO_BLK_F_TOPOLOGY  */
	/* exponent for physical block per logical block. */
	u8 physical_block_exp;
	/* alignment offset in logical blocks. */
	u8 alignment_offset;
	/* minimum I/O size without performance penalty in logical blocks. */
	u16 min_io_size;
	/* optimal sustained I/O size in logical blocks. */
	u32 opt_io_size;

	/* writeback mode (if VIRTIO_BLK_F_CONFIG_WCE) */
	u8 wce;
	u8 unused;

	/* number of vqs, only available when VIRTIO_BLK_F_MQ is set */
	u16 num_queues;

	/* the next 3 entries are guarded by VIRTIO_BLK_F_DISCARD */
	/*
	 * The maximum discard sectors (in 512-byte sectors) for
	 * one segment.
	 */
	u32 max_discard_sectors;
	/*
	 * The maximum number of discard segments in a
	 * discard command.
	 */
	u32 max_discard_seg;
	/* Discard commands must be aligned to this number of sectors. */
	u32 discard_sector_alignment;

	/* the next 3 entries are guarded by VIRTIO_BLK_F_WRITE_ZEROES */
	/*
	 * The maximum number of write zeroes sectors (in 512-byte sectors) in
	 * one segment.
	 */
	u32 max_write_zeroes_sectors;
	/*
	 * The maximum number of segments in a write zeroes
	 * command.
	 */
	u32 max_write_zeroes_seg;
	/*
	 * Set if a VIRTIO_BLK_T_WRITE_ZEROES request may result in the
	 * deallocation of one or more of the sectors.
	 */
	u8 write_zeroes_may_unmap;

	u8 unused1[3];

	/* the next 3 entries are guarded by VIRTIO_BLK_F_SECURE_ERASE */
	/*
	 * The maximum secure erase sectors (in 512-byte sectors) for
	 * one segment.
	 */
	u32 max_secure_erase_sectors;
	/*
	 * The maximum number of secure erase segments in a
	 * secure erase command.
	 */
	u32 max_secure_erase_seg;
	/* Secure erase commands must be aligned to this number of sectors. */
	u32 secure_erase_sector_alignment;

	/* Zoned block device characteristics (if VIRTIO_BLK_F_ZONED) */
	struct virtio_blk_zoned_characteristics {
		u32 zone_sectors;
		u32 max_open_zones;
		u32 max_active_zones;
		u32 max_append_sectors;
		u32 write_granularity;
		u8 model;
		u8 unused2[3];
	} zoned;
} __attribute__((packed));

