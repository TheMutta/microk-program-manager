#pragma once
#include <cdefs.h>
#include <object.hpp>

#include "memory.hpp"

struct RSDP_t {
	char Signature[8];
	u8 Checksum;
	char OEMID[6];
	u8 Revision;
	u32 RsdtAddress;

	u32 Length;
	u64 XsdtAddress;
	u8 ExtendedChecksum;
	u8 reserved[3];
} __attribute__ ((packed));

struct SDTHeader_t {
	char Signature[4];
	u32 Length;
	u8 Revision;
	u8 Checksum;
	char OEMID[6];
	char OEMTableID[8];
	u32 OEMRevision;
	u32 CreatorID;
	u32 CreatorRevision;
} __attribute__ ((packed));

struct MCFGEntry_t {
	uptr BaseAddress;
	u16 PCISeg;
	u8 StartPCIBus;
	u8 EndPCIBus;
	u8 Reserved[4];
}__attribute__((packed));

struct MCFG_t : public SDTHeader_t {
	u8 Reserved[8];

	MCFGEntry_t FirstEntry;
}__attribute__((packed));

struct PCIDeviceHeader_t {
	u16 VendorID;
	u16 DeviceID;
	u16 Command;
	u16 Status;
	u8 RevisionID;
	u8 ProgIF;
	u8 Subclass;
	u8 Class;
	u8 CacheLineSize;
	u8 LatencyTimer;
	u8 HeaderType;
	u8 BIST;
}__attribute__((packed));

struct PCIHeader0_t : public PCIDeviceHeader_t {
	u32 BAR0;
	u32 BAR1;
	u32 BAR2;
	u32 BAR3;
	u32 BAR4;
	u32 BAR5;
	u32 CardbusCISPointer;
	u16 SunbystemVendorID;
	u16 SubsystemID;
	u32 ExpansionROMBaseAddress;
	u8 CapabilitiesPointer;
	u8 Reserved0[3];
	u32 Reserved1;
	u8 InterruptLine;
	u8 InterruptPIN;
	u8 MinGrant;
	u8 MaxLatency;
}__attribute__((packed));

struct PCIHeader1_t : public PCIDeviceHeader_t {
	u32 BAR0;
	u32 BAR1;
	u8 PrimaryBusNumber;
	u8 SecondaryBusNumber;
	u8 SubordinateBusNumber;
	u8 SecondaryLatencyTimer;
	u8 IOBase;
	u8 IOLimit;
	u16 SecondaryStatus;
	u16 MemoryBase;
	u16 MemoryLimit;
	u16 PrefetchableMemoryBase;
	u16 PrefetchableMemoryLimit;
	u32 PrefetchableBaseUpper;
	u32 PrefetchableLimitUpper;
	u16 IOBaseUpper;
	u16 IOLimitUpper;
	u8 CapabilityPointer;
	u8 Reserved0[3];
	u32 ExpansionROMBaseAddress;
	u8 InterruptLine;
	u8 InterruptPIN;
	u16 BridgeControl;
}__attribute__((packed));

struct PCIHeader2_t : public PCIDeviceHeader_t {
	u32 CardBusBaseAddress;
	u8 CapabilityListOffset;
	u8 Reserved0;
	u16 SecondaryStatus;
	u8 PCIBusNumber;
	u8 CardBusNumber;
	u8 SubordinateBusNumber;
	u8 CardBusLatencyTimer;
	u32 MemoryBaseAddress0;
	u32 MemoryLimit0;
	u32 MemoryBaseAddress1;
	u32 MemoryLimit1;
	u32 IOBaseAddress0;
	u32 IOLimit0;
	u32 IOBaseAddress1;
	u32 IOLimit1;
	u8 InterruptLine;
	u8 InterruptPIN;
	u16 BridgeControl;
	u16 SubsystemDeviceID;
	u16 SubsystemVendorID;
	u32 LegacyModeBaseAddress;
}__attribute__((packed));

void InitACPI(MemoryMapper *mapper, ContainerInfo *info);
