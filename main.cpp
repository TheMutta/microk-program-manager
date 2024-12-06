#include <cpuid.h>
#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include <cdefs.h>
#include <object.hpp>

#include "capability.hpp"
#include "memory.hpp"

void ExceptionHandler(usize excp, usize errinfo1, usize errinfo2) {
	mkmi_log("Exception!\r\n");
	mkmi_log("%d -> 0x%x 0x%x\r\n", excp, errinfo1, errinfo2);

	while (true) { }
}

void InterruptHandler() {
	mkmi_log("Interrupt!\r\n");

	while (true) { }
}

void SyscallHandler() {
	mkmi_log("Syscall!\r\n");

	while (true) { }

}
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


__attribute__((used, section(".microkosm_bindings")))
static volatile ContainerBindings bindings = {
	.ExceptionHandler = ExceptionHandler,
	.InterruptHandler = InterruptHandler,
	.SyscallHandler = SyscallHandler,
};

void GetVendor() {
	u32 vendor[4] = {};
	u32 null = 0;
	__cpuid(0x40000000, null, vendor[0], vendor[1], vendor[2]);
	mkmi_log("Vendor hypervisor: 0x%x, %s\r\n", null, vendor);
}

extern "C" int Main(ContainerInfo *info) {
	mkmi_log("Hello from the containerized OS.\r\n");
	mkmi_log("Response 0x%x\r\n", info);
	mkmi_log("Initrd [0x%x-0x%x]\r\n", info->InitrdAddress, info->InitrdAddress + info->InitrdSize);
	mkmi_log("RSDP: 0x%x\r\n", info->x86_64.RSDPCapability);
	mkmi_log("DTB: 0x%x\r\n", info->DTB);

	GetVendor();

	Capability untyped;
	usize count;
	for (count = 0; ; ++count) {
		__fast_syscall(SYSCALL_VECTOR_GET_UNTYPED_CAPABILITY, (uptr)&untyped, count, 0, 0, 0, 0);
		if (untyped.Type != UNTYPED_FRAMES) break;
	}

	Capability untypedArray[count];
	bool usable[count];
	for (usize i = 0; i < count; ++i) {
		__fast_syscall(SYSCALL_VECTOR_GET_UNTYPED_CAPABILITY, (uptr)&untypedArray[i], i, 0, 0, 0, 0);
		mkmi_log("Got region [0x%x - 0x%x]\r\n", untypedArray[i].Object, untypedArray[i].Object + untypedArray[i].Size);
		if (untypedArray[i].Type != UNTYPED_FRAMES) { usable[i] = false; break; };
		usable[i] = true;
	}
		
	mkmi_log("Total of %d capabilities available\r\n", count);

	InitializeUntypedMemory(untypedArray, usable, count);

	Capability framesUt;
	Capability levelsUt;
	GetUntypedRegion(PAGE_SIZE * 3, &levelsUt);
	GetUntypedRegion(PAGE_SIZE * 64, &framesUt);

	mkmi_log("Got region [0x%x - 0x%x]\r\n", levelsUt.Object, levelsUt.Object + levelsUt.Size);
	mkmi_log("Got region [0x%x - 0x%x]\r\n", framesUt.Object, framesUt.Object + framesUt.Size);


	Capability framesF[64];
	Capability levelsVPS[3];

	RetypeCapability(framesUt, framesF, FRAME_MEMORY, 64);
	RetypeCapability(levelsUt, levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 3);

	mkmi_log("Got region [0x%x - 0x%x]\r\n", levelsVPS[0].Object, levelsVPS[0].Object + levelsVPS[0].Size);
	mkmi_log("Got region [0x%x - 0x%x]\r\n", levelsVPS[1].Object, levelsVPS[1].Object + levelsVPS[1].Size);
	mkmi_log("Got region [0x%x - 0x%x]\r\n", levelsVPS[2].Object, levelsVPS[2].Object + levelsVPS[2].Size);
	mkmi_log("Got region [0x%x - 0x%x]\r\n", framesF[0].Object, framesF[0].Object + framesF[0].Size);

	uptr addr = 0x1000;

	mkmi_log("Returned: %d\r\n", MMapIntermediate(levelsVPS[2], 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	mkmi_log("Returned: %d\r\n", MMapIntermediate(levelsVPS[1], 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	mkmi_log("Returned: %d\r\n", MMapIntermediate(levelsVPS[0], 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	for (int i = 0; i < 64; ++i) {
		mkmi_log("Returned: %d\r\n", MMapPage(framesF[i], addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	}

	mkmi_log("Trying to access page...\r\n");
	*(u32*)(addr) = 0xDEAD;
	mkmi_log("Result: 0x%x\r\n", *(u32*)addr);
	
	Heap kernelHeap(addr, 64 * PAGE_SIZE);
	MemoryMapper memoryMapper(0x400000000);

	Capability rsdpCapability;
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, info->x86_64.RSDPCapability, (uptr)&rsdpCapability, 0, 0, 0, 0);

	void *rsdpAddr = memoryMapper.MMap(rsdpCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	RSDP_t *rsdp = (RSDP_t*)((uptr)rsdpAddr + info->x86_64.RSDPOffset);
	mkmi_log("rsdp at 0x%x\r\n", rsdp);

	{
		char signature[9] = { '\0' };
		for (int i = 0; i < 8; ++i) {
			signature[i] = rsdp->Signature[i];
		}
	
		mkmi_log("Signature: %s\r\n", signature);
	}

	mkmi_log("RSDT: 0x%x\r\n", rsdp->RsdtAddress);
	mkmi_log("XSDT: 0x%x\r\n", rsdp->XsdtAddress);

	uptr sdtAddr = rsdp->XsdtAddress;
	ROUND_DOWN_TO(sdtAddr, PAGE_SIZE);

	Capability xsdtCapability;
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, sdtAddr, (uptr)&xsdtCapability, 0, 0, 0, 0);
	void *xsdtAddr = memoryMapper.MMap(xsdtCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	SDTHeader_t *xsdt = (SDTHeader_t*)((uptr)xsdtAddr + (rsdp->XsdtAddress % PAGE_SIZE));

	{
		char signature[5] = { '\0' };
		for (int i = 0; i < 4; ++i) {
			signature[i] = xsdt->Signature[i];
		}
		
		mkmi_log("Signature: %s\r\n", signature);
	}

	int entries = (xsdt->Length - sizeof(SDTHeader_t)) / 8;

	for (int i = 0; i < entries; i++) {
		char sig[5] = { '\0' };
		uptr *ptr = (uptr*)((uptr)xsdt + sizeof(SDTHeader_t) + i * 8);

		mkmi_log(" -> 0x%x\r\n", ptr);

		Capability sdtCapability;
		__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, *ptr, (uptr)&sdtCapability, 0, 0, 0, 0);
		void *sdtAddr = memoryMapper.MMap(sdtCapability, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
		SDTHeader_t *sdt = (SDTHeader_t*)((uptr)sdtAddr);

		memcpy(sig, sdt->Signature, 4);
		mkmi_log("%d: 0x%x -> %s\r\n", i, *ptr, sig);
	}




/*
	uptr xsdtRoundedAddr = rsdp->XsdtAddress - rsdp->XsdtAddress % PAGE_SIZE;
	Capability xsdtCapability;
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, xsdtRoundedAddr, (uptr)&xsdtCapability, 0, 0, 0, 0);

	Capability capability;
	uptr capabilityAddr;
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, untypedArray[largestCapabilityIndex].Object, UNTYPED_FRAMES, (uptr)&capability, (uptr)&capabilityAddr, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capabilityAddr, capability.Object);
	mkmi_log("Capability has size of %d bytes or %d pages.\r\n", capability.Size, capability.Size / PAGE_SIZE);

	usize splitCount = 16 + 3;
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

	mkmi_log("Returned: %d\r\n", MMapIntermediate(levels[2], 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	mkmi_log("Returned: %d\r\n", MMapIntermediate(levels[1], 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	mkmi_log("Returned: %d\r\n", MMapIntermediate(levels[0], 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));

	for (usize i = 0; i < pages; ++i) {
		mkmi_log("Returned: %d\r\n", MMapPage(frame[i], addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE));
	}

	VirtualMemoryHeader heapHeader = headers[0];
	mkmi_log("Covered [0x%x - 0x%x]\r\n", heapHeader.LVL1Start, heapHeader.LVL1Start + heapHeader.LVL1Length);
	mkmi_log("Covered [0x%x - 0x%x]\r\n", heapHeader.LVL2Start, heapHeader.LVL2Start + heapHeader.LVL2Length);
	mkmi_log("Covered [0x%x - 0x%x]\r\n", heapHeader.LVL3Start, heapHeader.LVL3Start + heapHeader.LVL3Length);


	mkmi_log("Trying to access page...\r\n");
	*(u32*)(addr) = 0xDEAD;
	mkmi_log("Result: 0x%x\r\n", *(u32*)addr);


	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capabilityAddr, apicCapability.Object);

	uptr apicMap = addr + pages * PAGE_SIZE;
	__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, apicCapability.Object, MMIO_MEMORY, apicMap, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);


*/

	return 0;
}

