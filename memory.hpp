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

extern VirtualMemoryHeader headers[MAX_MEMORY_HEADERS];

#define MMAP_OK 0
#define MMAP_ERR  128
#define MMAP_LVL3 3
#define MMAP_LVL2 2
#define MMAP_LVL1 1

int MMapIntermediate(Capability cap, usize level, uptr addr, usize flags);
int MMap(Capability cap, uptr addr, usize flags);
