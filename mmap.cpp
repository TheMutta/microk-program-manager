#include <mkmi.h>
#include "memory.hpp"
#include "capability.hpp"

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

void *MemoryMapper::MMap(Capability capability, usize flags) {
	uptr addr = StartAddr;
	for (usize i = 0; i < capability.Size; i += PAGE_SIZE) {
		if (addr + i >= LVL3Coverage) {
			Capability levelsUt;
			Capability levelsVPS[3];

			GetUntypedRegion(PAGE_SIZE * 3, &levelsUt);
			RetypeCapability(levelsUt, levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 3);

			MMapIntermediate(levelsVPS[2], 3, addr + i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			MMapIntermediate(levelsVPS[1], 2, addr + i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			MMapIntermediate(levelsVPS[0], 1, addr + i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

			LVL3Coverage += MAX_LVL3_LENGTH;
			LVL2Coverage += MAX_LVL2_LENGTH;
			LVL1Coverage += MAX_LVL1_LENGTH;
		} else if (addr + i >= LVL2Coverage) {
			Capability levelsUt;
			Capability levelsVPS[2];

			GetUntypedRegion(PAGE_SIZE * 2, &levelsUt);
			RetypeCapability(levelsUt, levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 2);

			MMapIntermediate(levelsVPS[1], 2, addr + i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
			MMapIntermediate(levelsVPS[0], 1, addr + i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

			LVL2Coverage += MAX_LVL2_LENGTH;
			LVL1Coverage += MAX_LVL1_LENGTH;
		} else if (addr + i >= LVL1Coverage) {
			Capability levelsUt;
			Capability levelsVPS;

			GetUntypedRegion(PAGE_SIZE, &levelsUt);
			RetypeCapability(levelsUt, &levelsVPS, VIRTUAL_MEMORY_PAGING_STRUCTURE, 1);

			MMapIntermediate(levelsVPS, 1, addr + i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);

			LVL1Coverage += MAX_LVL1_LENGTH;
		}

		MMapPage(capability, addr + i, flags);
		StartAddr += PAGE_SIZE;
	}

	return (void*)addr;
}

void MemoryMapper::UnMap(void *ptr) {

}
