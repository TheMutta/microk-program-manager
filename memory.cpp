#include <mkmi.h>

#include "memory.hpp"
#include "capability.hpp"


static Capability *MemoryMap;
static bool *Usable;
static usize RegionCount;

void InitializeUntypedMemory(Capability *map, bool *usable, usize count) {
	MemoryMap = map;
	RegionCount = count;
	Usable = usable;
}



int GetUntypedRegion(usize size, Capability *capability) {
	// SIZE IS ROUNDED TO PAGE !!!

	for (int i = 0; i < RegionCount; ++i) {
		if(!Usable[i]) continue;

		Capability region = MemoryMap[i];

		if (region.Size > size) {
			uptr nextAddr = region.Object + size;
			SplitCapability(region, capability, 1, size);
			AddressCapability(nextAddr, &MemoryMap[i]);
			return GUNTPD_OK;
		} else if (region.Size == size) {
			Usable[i] = false;
			*capability = region;
			return GUNTPD_OK;
		} else {
			continue;
		}
	}

	return GUNTPD_NOMEM;
}

