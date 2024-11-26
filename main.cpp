#include <cpuid.h>
#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include <cdefs.h>

#define HIGHER 0xFFFFFF8000000000
#define PAGE_SIZE 4096
	
struct Capability {
	u8 IsMasked : 1;
	u8 Type : 7;
	u32 Children : 24;

	u16 AccessRights;
	u16 AccessRightsMask;

	uptr Object;
	Capability *Parent;
	// TODO: overhaul parent/child relationship
}__attribute__((packed, aligned(0x10)));

extern "C" int Main() {
	mkmi_log("Hello from the containerized OS.\r\n");

	u32 vendor[4] = {};
	u32 null = 0;
	__cpuid(0x40000000, null, vendor[0], vendor[1], vendor[2]);
	mkmi_log("Vendor hypervisor: 0x%x, %s\r\n", null, vendor);

	Capability capability;
	uptr capabilityAddr;
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, 0, UNTYPED_FRAMES, (uptr)&capability, (uptr)&capabilityAddr, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x with %d children\r\n", capabilityAddr, capability.Object, capability.Children);

	usize splitCount = 2;
	Capability capabilities[splitCount];
	__fast_syscall(SYSCALL_VECTOR_SPLIT_CAPABILITY, capability.Object, (uptr)capabilities, PAGE_SIZE, splitCount, 0, 0);
	for (usize i = 0; i < splitCount; ++i) {
		mkmi_log("Capability: 0x%x -> 0x%x with %d children\r\n", capabilityAddr, capabilities[i].Object, capabilities[i].Children);
	}

/*
	__fast_syscall(SYSCALL_VECTOR_RETYPE_CAPABILITY, capabilityAddr, (uptr)&capability, PAGE_SIZE, 1, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x with %d children\r\n", capabilityAddr, capability.Object, capability.Children);
*/
	return 0;
}

