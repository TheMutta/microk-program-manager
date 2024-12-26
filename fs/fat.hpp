#pragma once
#include <cdefs.h>

struct FATHeader_t {
	u8 Reserved0[3];
	u8 OEMID[8];
	u16 BytesPerSector;
	u8 SectorsPerCluster;
	u16 ReservedSectors;
	u8 FATCount;
	u16 RootDirectoryEntries;
	u16 TotalSectors;
	u8 MediaDescriptorType;
	u16 SectorPerFAT16;
	u16 SectorsPerTrack;
	u16 Heads;
	u32 HiddenSectors;
	u32 LargeSectorCount;
}__attribute__((packed));

struct FAT32Header_t : public FATHeader_t {
	u32 SectorsPerFAT;
	u16 Flags;
	u16 VersionNumber;
	u32 RootDirCluster;
	u16 FSInfoSector;
	u16 BackupBootsecSector;
	u8 Reserved1[12];
	u8 DriveNumber;
	u8 Reserved2;
	u8 Signature;
	u32 VolumeID;
	u8 VolumeLabel[11];
	u8 SystemIdentifier[8];
	u8 BootCode[420];
	u16 BootableSignature;
}__attribute__((packed));

struct FAT32FSInfo_t {
	u32 Signature;
}__attribute__((packed));

struct EXFATHeader_t {
	u8 Reserved0[3];
	u8 OEMID[8];
	u8 Reserved1[53];
	u64 PartitionOffset;
	u64 VolumeLength;
	u32 FATOffset;
	u32 FATLength;
	u32 ClusterHeapOffset;
	u32 ClusterCount;
	u32 RootDirectoryCluster;
	u32 PartitionSerialNumber;
	u16 FSRevision;
	u16 Flags;
	u8 SectorShift;
	u8 ClusterSHift;
	u8 FATNumber;
	u8 DriveSelect;
	u8 PercentageUse;
	u8 Reserved2[7];
}__attribute__((packed));
