#include <mkmi.h>
#include "memory.hpp"
#include "../init/capability.hpp"

MemoryMapper::MemoryMapper(uptr startAddr) : StartAddr(startAddr) {
	Capability levelsUt;
	Capability levelsVPS[3];

	GetUntypedRegion(PAGE_SIZE * 3, &levelsUt);
	RetypeCapability(levelsUt, levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 3);

	MMapIntermediate(levelsVPS[2], 3, StartAddr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	MMapIntermediate(levelsVPS[1], 2, StartAddr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	MMapIntermediate(levelsVPS[0], 1, StartAddr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

	LVL4Coverage = MAX_LVL4_LENGTH + startAddr;
	LVL3Coverage = MAX_LVL3_LENGTH + startAddr;
	LVL2Coverage = MAX_LVL2_LENGTH + startAddr;
	LVL1Coverage = MAX_LVL1_LENGTH + startAddr;
}

void *MemoryMapper::MMap(Capability *capabilityArray, usize count, usize flags) {
	mkmi_log("Called MMap for array 0x%x of count %d\r\n", capabilityArray, count);

	uptr addr = StartAddr;
	for (usize i = 0; i < count ; ++i) {
		if (addr + (i + 1) * PAGE_SIZE >= LVL3Coverage) {
			Capability levelsUt;
			Capability levelsVPS[3];

			GetUntypedRegion(PAGE_SIZE * 3, &levelsUt);
			RetypeCapability(levelsUt, levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 3);

			MMapIntermediate(levelsVPS[2], 3, addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			MMapIntermediate(levelsVPS[1], 2, addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			MMapIntermediate(levelsVPS[0], 1, addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

			LVL3Coverage += MAX_LVL3_LENGTH;
			LVL2Coverage += MAX_LVL2_LENGTH;
			LVL1Coverage += MAX_LVL1_LENGTH;
		} else if (addr + (i + 1) * PAGE_SIZE >= LVL2Coverage) {
			Capability levelsUt;
			Capability levelsVPS[2];

			GetUntypedRegion(PAGE_SIZE * 2, &levelsUt);
			RetypeCapability(levelsUt, levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 2);

			MMapIntermediate(levelsVPS[1], 2, addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			MMapIntermediate(levelsVPS[0], 1, addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

			LVL2Coverage += MAX_LVL2_LENGTH;
			LVL1Coverage += MAX_LVL1_LENGTH;
		} else if (addr + (i + 1) * PAGE_SIZE >= LVL1Coverage) {
			Capability levelsUt;
			Capability levelsVPS;

			GetUntypedRegion(PAGE_SIZE, &levelsUt);
			RetypeCapability(levelsUt, &levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 1);

			MMapIntermediate(levelsVPS, 1, addr + i * PAGE_SIZE, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

			LVL1Coverage += MAX_LVL1_LENGTH;
		}

		MMapPage(capabilityArray[i], addr + i * PAGE_SIZE, flags);
		StartAddr += PAGE_SIZE;
	}

	return (void*)addr;
}

void MemoryMapper::MUnmap(Capability *capabilityArray, usize count) {


}
