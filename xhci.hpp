#pragma once
#include <cdefs.h>

#include "memory.hpp"
#include "acpi.hpp"

struct XHCICapabilityRegs_t {
	u8 Length;
	u8 Reserved0;
	u16 InterfaceVersion;
	u32 StructuralParameters1;
	u32 StructuralParameters2;
	u32 StructuralParameters3;
	u32 CapabilityParameters1;
	u32 DorbellOffset;
	u32 RuntimeRegsSpaceOffset;
	u32 CapabilityParameters2;
}__attribute__((packed));

struct XHCIOperationalRegs_t {
	u32 USBCommand;
	u32 USBStatus;
	u32 PageSize;
	u8 Reserved0[8];
	u32 DeviceNotificationCtl;
	u64 CommandRingControl;
	u8 Reserved1[16];
	u64 DeviceContextBaseArray;
	u32 Configure;
}__attribute__((packed));

struct XHCIPortRegs_t {
	u32 StatusCtl;
	u32 PowerManagmentStatusCtl;
	u32 LinkInfo;
	u32 Reserved0;
}__attribute__((packed));

struct XHCIPortRuntimeRegs {

}__attribute__((packed));

void InitializeXHCIDevice(Heap *kernelHeap, MemoryMapper *mapper, PCIHeader0_t *header0, PCICapability_t *pciCapabilityArray, usize pciCapabilityCount);
