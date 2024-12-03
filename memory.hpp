#include <cdefs.h>
#include <object.hpp>

#define MAX_MEMORY_HEADERS 256ULL
#define MAX_LVL1_LENGTH (512ULL * PAGE_SIZE)
#define MAX_LVL2_LENGTH (512ULL * MAX_LVL1_LENGTH)
#define MAX_LVL3_LENGTH (512ULL * MAX_LVL2_LENGTH)

struct VirtualMemoryHeader {
	uptr LVL3Start;
	usize LVL3Length;
	uptr LVL2Start;
	usize LVL2Length;
	uptr LVL1Start;
	usize LVL1Length;
};

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
