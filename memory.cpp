#include "memory.hpp"
#include <mkmi.h>

void MapIntermediate(Capability capability, usize level, uptr addr, usize flags);
void MapPage(Capability frame, uptr addr, usize flags);

VirtualMemoryHeader headers[MAX_MEMORY_HEADERS];

int MMap(Capability cap, uptr addr, usize flags) {
	VirtualMemoryHeader *header = &headers[addr / MAX_LVL3_LENGTH];

	if (header->LVL3Start < addr || header->LVL3Start + header->LVL3Length < addr + cap.Size || header->LVL3Length == 0) return -MMAP_LVL3;
	if (header->LVL2Start < addr || header->LVL2Start + header->LVL2Length < addr + cap.Size || header->LVL2Length == 0) return -MMAP_LVL2;
	if (header->LVL1Start < addr || header->LVL1Start + header->LVL1Length < addr + cap.Size || header->LVL1Length == 0) return -MMAP_LVL1;

	MapPage(cap, addr, flags);

	return MMAP_OK;

}

int MMapIntermediate(Capability cap, usize level, uptr addr, usize flags) {
	VirtualMemoryHeader *header = &headers[addr / MAX_LVL3_LENGTH];

	if (level == 3) {
		MapIntermediate(cap, level, addr, flags);
		header->LVL3Start = (addr >> 39) << 39;
		header->LVL3Length = MAX_LVL3_LENGTH;

		return MMAP_OK;
	}

	if (header->LVL3Length == 0) return -MMAP_LVL3;

	if (level == 2) {
		MapIntermediate(cap, level, addr, flags);
		header->LVL2Start = (addr >> 30) << 30;
		header->LVL2Length = MAX_LVL2_LENGTH;

		return MMAP_OK;
	}

	if (header->LVL2Start < addr || header->LVL2Start + header->LVL2Length < addr + cap.Size || header->LVL2Length == 0) return -MMAP_LVL2;

	if (level == 1) {
		MapIntermediate(cap, level, addr, flags);
		header->LVL1Start = (addr >> 21) << 21;
		header->LVL1Length = MAX_LVL1_LENGTH;

		return MMAP_OK;
	}

	if (header->LVL1Start < addr || header->LVL1Start + header->LVL1Length < addr + cap.Size || header->LVL1Length == 0) return -MMAP_LVL1;

	return MMAP_ERR;

}

void MapIntermediate(Capability capability, usize level, uptr addr, usize flags) {
	
	__fast_syscall(SYSCALL_VECTOR_MAP_INTERMEDIATE_CAPABILITY, capability.Object, level, addr, flags , 0, 0);
}

void MapPage(Capability frame, uptr addr, usize flags) {
	__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, frame.Object, FRAME_MEMORY, addr, flags , 0, 0);
}
