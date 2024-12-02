#include <cpuid.h>
#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include <cdefs.h>
#include <object.hpp>

#include "memory.hpp"

struct RSDP_t {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;

	uint32_t Length;
	uint64_t XsdtAddress;
	uint8_t ExtendedChecksum;
	uint8_t reserved[3];
} __attribute__ ((packed));

struct SDTHeader_t {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
} __attribute__ ((packed));

void ExceptionHandler(usize excp, usize errinfo1, usize errinfo2) {
	mkmi_log("Exception!\r\n");
	mkmi_log("%d -> 0x%x 0x%x\r\n", excp, errinfo1, errinfo2);

	while (true) { }
}

void InterruptHandler() {

}

void SyscallHandler() {

}


__attribute__((used, section(".microkosm_bindings")))
static volatile ContainerBindings bindings = {
	.ExceptionHandler = ExceptionHandler,
	.InterruptHandler = InterruptHandler,
	.SyscallHandler = SyscallHandler,
};

void SplitCapability(Capability capability, Capability *capabilities, usize splitCount) {
	__fast_syscall(SYSCALL_VECTOR_SPLIT_CAPABILITY, capability.Object, (uptr)capabilities, PAGE_SIZE, 0, splitCount, 0);
	for (usize i = 0; i < splitCount; ++i) {
		mkmi_log("Capability: 0x%x\r\n", capabilities[i].Object);
	}
}

void RetypeCapability(Capability capability, Capability *result, OBJECT_TYPE kind) {
	__fast_syscall(SYSCALL_VECTOR_RETYPE_CAPABILITY, capability.Object, kind, (uptr)result, 1,  0, 0);
	mkmi_log("Capability: 0x%x\r\n", result->Object);
}



extern "C" int Main(uptr rsdp) {
	mkmi_log("Hello from the containerized OS.\r\n");

	u32 vendor[4] = {};
	u32 null = 0;
	__cpuid(0x40000000, null, vendor[0], vendor[1], vendor[2]);
	mkmi_log("Vendor hypervisor: 0x%x, %s\r\n", null, vendor);

	Capability capability;
	uptr capabilityAddr;
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, 0, UNTYPED_FRAMES, (uptr)&capability, (uptr)&capabilityAddr, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capabilityAddr, capability.Object);
	mkmi_log("Capability has size of %d bytes or %d pages.\r\n", capability.Size, capability.Size / PAGE_SIZE);

	usize splitCount = capability.Size / PAGE_SIZE;
	usize pages = splitCount - 3;

	Capability capabilities[splitCount];
	SplitCapability(capability, capabilities, splitCount);

	Capability frame[pages];
	Capability levels[3];
	for (usize i = 0; i < pages; ++i) {
		RetypeCapability(capabilities[i], &frame[i], FRAME_MEMORY);
	}

	for (int i = 0; i < 3; ++i) {
		RetypeCapability(capabilities[i + pages], &levels[i], VIRTUAL_MEMORY_PAGING_STRUCTURE);
		mkmi_log("Capability: 0x%x -> 0x%x\r\n", capabilityAddr, levels[i].Object);
	}
	
	uptr addr = 0;

	
	

	MMapIntermediate(levels[2], 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	MMapIntermediate(levels[1], 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	MMapIntermediate(levels[0], 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	for (usize i = 0; i < pages; ++i) {
		MMap(frame[i], addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	}

	VirtualMemoryHeader heapHeader = headers[0];
	mkmi_log("Covered [0x%x - 0x%x]\r\n", heapHeader.LVL1Start, heapHeader.LVL1Start + heapHeader.LVL1Length);
	mkmi_log("Covered [0x%x - 0x%x]\r\n", heapHeader.LVL2Start, heapHeader.LVL2Start + heapHeader.LVL2Length);
	mkmi_log("Covered [0x%x - 0x%x]\r\n", heapHeader.LVL3Start, heapHeader.LVL3Start + heapHeader.LVL3Length);


	mkmi_log("Trying to access page...\r\n");
	*(u32*)addr = 0xDEAD;
	mkmi_log("Result: 0x%x\r\n", *(u32*)addr);

	uptr apic = 0xFEE00000;
	mkmi_log ("Apic at: 0x%x\r\n", apic);

	Capability apicCapability;
	__fast_syscall(SYSCALL_VECTOR_CREATE_FROM_MEM_CAPABILITY, apic, (uptr)&apicCapability, 0, 0, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capabilityAddr, apicCapability.Object);

	uptr apicMap = addr + pages * PAGE_SIZE;
	__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, apicCapability.Object, MMIO_MEMORY, apicMap, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);

	mkmi_log("Apic id: %x\r\n", *(u8*)(apicMap + 0x20));
	mkmi_log("Apic ver: %x\r\n", *(u8*)(apicMap + 0x30));

	uptr rsdpMap = apicMap + PAGE_SIZE;
	mkmi_log("rsdp at: 0x%x\r\n", rsdp);
	Capability rsdpCapability;
	RSDP_t *rsdpTable = NULL;
	if (rsdp % PAGE_SIZE) {
		uptr rsdpAddr = rsdp - rsdp % PAGE_SIZE;
		__fast_syscall(SYSCALL_VECTOR_CREATE_FROM_MEM_CAPABILITY, rsdpAddr, (uptr)&rsdpCapability, 0, 0, 0, 0);
		__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, rsdpCapability.Object, MMIO_MEMORY, rsdpMap, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);
		rsdpTable = (RSDP_t*)(rsdpMap + (rsdp - rsdpAddr));
	} else {
		__fast_syscall(SYSCALL_VECTOR_CREATE_FROM_MEM_CAPABILITY, rsdp, (uptr)&rsdpCapability, 0, 0, 0, 0);
		__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, rsdpCapability.Object, MMIO_MEMORY, rsdpMap, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);
		rsdpTable = (RSDP_t*)rsdpMap;
	}

	mkmi_log("RSDP at: 0x%x\r\n", rsdpTable);

	return 0;
}

