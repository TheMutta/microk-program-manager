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

struct AddressStructure_t {
	u8 AddressSpaceID;    // 0 - system memory, 1 - system I/O
	u8 RegisterBitWidth;
	u8 RegisterBitOffset;
	u8 Reserved0;
	u64 Address;
}__attribute__((packed));

struct FADT_t : public SDTHeader_t {
	u32 FirmwareCtrl;
	u32 Dsdt;

	// field used in ACPI 1.0; no longer in use, for compatibility only
	u8  Reserved;

	u8  PreferredPowerManagementProfile;
	u16 SCI_Interrupt;
	u32 SMI_CommandPort;
	u8  AcpiEnable;
	u8  AcpiDisable;
	u8  S4BIOS_REQ;
	u8  PSTATE_Control;
	u32 PM1aEventBlock;
	u32 PM1bEventBlock;
	u32 PM1aControlBlock;
	u32 PM1bControlBlock;
	u32 PM2ControlBlock;
	u32 PMTimerBlock;
	u32 GPE0Block;
	u32 GPE1Block;
	u8  PM1EventLength;
	u8  PM1ControlLength;
	u8  PM2ControlLength;
	u8  PMTimerLength;
	u8  GPE0Length;
	u8  GPE1Length;
	u8  GPE1Base;
	u8  CStateControl;
	u16 WorstC2Latency;
	u16 WorstC3Latency;
	u16 FlushSize;
	u16 FlushStride;
	u8  DutyOffset;
	u8  DutyWidth;
	u8  DayAlarm;
	u8  MonthAlarm;
	u8  Century;

	// reserved in ACPI 1.0; used since ACPI 2.0+
	u16 BootArchitectureFlags;

	u8  Reserved2;
	u32 Flags;

	// 12 byte structure; see below for details
	AddressStructure_t ResetReg;

	u8  ResetValue;
	u8  Reserved3[3];

	// 64bit pointers - Available on ACPI 2.0+
	u64                X_FirmwareControl;
	u64                X_Dsdt;

	AddressStructure_t X_PM1aEventBlock;
	AddressStructure_t X_PM1bEventBlock;
	AddressStructure_t X_PM1aControlBlock;
	AddressStructure_t X_PM1bControlBlock;
	AddressStructure_t X_PM2ControlBlock;
	AddressStructure_t X_PMTimerBlock;
	AddressStructure_t X_GPE0Block;
	AddressStructure_t X_GPE1Block;
}__attribute__((packed));

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
	u32 BAR[6];
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
	u32 BAR[2];
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

struct PCICapability_t {
	u8 CapID; /* Generic PCI field: PCI_CAP_ID_VNDR */
	u8 CapNext; /* Generic PCI field: next ptr. */
	u8 CapLength; /* Generic PCI field: capability length */
	u8 CfgType;
	u8 BAR;
	u8 Padding[3];
	u32 Offset;
	u32 Length;
}__attribute__((packed));

void InitACPI(Heap *kernelHeap, MemoryMapper *mapper, ContainerInfo *info);

uptr GetBAR(u32 bar, u32 nextBar);
uptr GetBAR(u32 bar, u32 nextBar, usize *size);
