#include <mkmi.h>
#include "memory.hpp"

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

	if (level == 2) {
		MapIntermediate(cap, level, addr, flags);
		return MMAP_OK;
	}

	if (level == 1) {
		MapIntermediate(cap, level, addr, flags);
		return MMAP_OK;
	}

	return MMAP_ERR;

}

void MapIntermediate(Capability capability, usize level, uptr addr, usize flags) {
	__fast_syscall(SYSCALL_VECTOR_MAP_INTERMEDIATE_CAPABILITY, capability.Object, level, addr, flags , 0, 0);
}

void MapPage(Capability frame, uptr addr, usize flags) {
	__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, frame.Object, FRAME_MEMORY, addr, flags , 0, 0);
}
