#pragma once
#include <cdefs.h>
#include <object.hpp>

#define MAX_LVL1_LENGTH (512ULL * PAGE_SIZE)
#define MAX_LVL2_LENGTH (512ULL * MAX_LVL1_LENGTH)
#define MAX_LVL3_LENGTH (512ULL * MAX_LVL2_LENGTH)
#define MAX_LVL4_LENGTH (512ULL * MAX_LVL3_LENGTH)

void InitializeUntypedMemory(Capability *map, bool *usable, usize count);

#define GUNTPD_OK 0
#define GUNTPD_NOMEM 1
int GetUntypedRegion(usize size, Capability *capability);

#define MMAP_OK 0
#define MMAP_ERR  128
#define MMAP_LVL3 3
#define MMAP_LVL2 2
#define MMAP_LVL1 1
int MMapIntermediate(Capability cap, usize level, uptr addr, usize flags);
int MMapPage(Capability cap, uptr addr, usize flags);

class Heap {
public:
	struct HeapBlock {
		HeapBlock *Next, *Previous;
		usize Size;
		bool IsFree;
	};

	Heap(uptr address, usize initialSize);
	void *Malloc(usize size);
	void Free(void *ptr);
	void ExpandHeap(usize amount);

	void DebugDump();
private:
	HeapBlock *RootBlock;

	uptr Address;
	usize Size;
};

/**
 * MMap when called gives some memory from a specific address
 * The kernel heap holds the allocated regions for MMap;
 */
class MemoryMapper {
public:
	struct MMapBlock {
		MMapBlock *Next, *Previous;
		uptr Addr;
		usize Size;
	};

	MemoryMapper(uptr startAddr);
	void *MMap(Capability capability, usize flags);
	void UnMap(void *ptr);

	uptr StartAddr;

	uptr LVL4Coverage;
	uptr LVL3Coverage;
	uptr LVL2Coverage;
	uptr LVL1Coverage;

};
