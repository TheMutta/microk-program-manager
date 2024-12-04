#include <mkmi.h>
#include "memory.hpp"

// TODO: Fix buggy checks

static void MapIntermediate(Capability capability, usize level, uptr addr, usize flags);
static void MapPage(Capability frame, uptr addr, usize flags);

int MMapPage(Capability cap, uptr addr, usize flags) {
	MapPage(cap, addr, flags);

	return MMAP_OK;

}

int MMapIntermediate(Capability cap, usize level, uptr addr, usize flags) {
	if (level == 3) {
		MapIntermediate(cap, level, addr, flags);
		return MMAP_OK;
	}

	//if (header->LVL3Length == 0) return -MMAP_LVL3;

	if (level == 2) {
		MapIntermediate(cap, level, addr, flags);
		return MMAP_OK;
	}

	//if (header->LVL2Start < addr || header->LVL2Start + header->LVL2Length < addr + cap.Size || header->LVL2Length == 0) return -MMAP_LVL2;

	if (level == 1) {
		MapIntermediate(cap, level, addr, flags);
		return MMAP_OK;
	}

	//if (header->LVL1Start < addr || header->LVL1Start + header->LVL1Length < addr + cap.Size || header->LVL1Length == 0) return -MMAP_LVL1;

	return MMAP_ERR;

}

void MapIntermediate(Capability capability, usize level, uptr addr, usize flags) {
	__fast_syscall(SYSCALL_VECTOR_MAP_INTERMEDIATE_CAPABILITY, capability.Object, level, addr, flags , 0, 0);
}

void MapPage(Capability frame, uptr addr, usize flags) {
	__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, frame.Object, FRAME_MEMORY, addr, flags , 0, 0);
}
