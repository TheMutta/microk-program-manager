#pragma once
#include <cdefs.h>

struct _GUID {
	union {
		struct {
			u32 Data1;
			u16 Data2;
			u16 Data3;
			u64 Data4;
		} Groups;
		u8 Data[16];
	};
}__attribute__((packed));

struct GPTHeader_t {
	u8 Signature[8];
	u32 GPTRevision;
	u32 HeaderSize;
	u32 CRC32Checksum;
	u32 Reserved;
	u64 ThisLBA;
	u64 AlternateLBA;
	u64 FirstUsableBlock;
	u64 LastUsableBlock;
	_GUID GUID;
	u64 ArrayStartLBA;
	u32 PartitionEntries;
	u32 PartitionEntrySize;
	u32 CRC32PartitionArray;
}__attribute__((packed));

struct GPTEntry_t {
	_GUID TypeGUID;
	_GUID UniqueGUID[16];
	u64 StartLBA;
	u64 EndLBA;
	u64 Attributes;
	u8 PartitionName[72];
}__attribute__((packed));
