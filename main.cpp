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

struct ContainerBindings {
	void (*ExceptionHandler)(usize excp, usize errinfo1, usize errinfo2);
	void (*InterruptHandler)();
	void (*SyscallHandler)();
}__attribute__((packed));

void ExceptionHandler(usize excp, usize errinfo1, usize errinfo2) {
	mkmi_log("Exception!\r\n");
	mkmi_log("%d -> 0x%x 0x%x\r\n", excp, errinfo1, errinfo2);

	while (true) { }
}

void InterruptHandler() {

}

void SyscallHandler() {

}

__attribute__((used, section(".microkosm_bindings")))
static volatile ContainerBindings bindings = {
	.ExceptionHandler = ExceptionHandler,
	.InterruptHandler = InterruptHandler,
	.SyscallHandler = SyscallHandler,
};

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

	usize splitCount = 3;
	Capability capabilities[splitCount];
	__fast_syscall(SYSCALL_VECTOR_SPLIT_CAPABILITY, capability.Object, (uptr)capabilities, PAGE_SIZE, splitCount, 0, 0);
	for (usize i = 0; i < splitCount; ++i) {
		mkmi_log("Capability: 0x%x -> 0x%x with %d children\r\n", capabilityAddr, capabilities[i].Object, capabilities[i].Children);
	}

	Capability frame;
	Capability levels[3];
	(void)levels;
	__fast_syscall(SYSCALL_VECTOR_RETYPE_CAPABILITY, capabilities[0].Object, FRAME_MEMORY, (uptr)&frame, 1,  0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x with %d children\r\n", capabilityAddr, frame.Object, frame.Children);

	for (int i = 0; i < 3; ++i) {
		__fast_syscall(SYSCALL_VECTOR_RETYPE_CAPABILITY, capabilities[i + 1].Object, VIRTUAL_MEMORY_PAGING_STRUCTURE, (uptr)&levels[i], 1,  0, 0);
		mkmi_log("Capability: 0x%x -> 0x%x with %d children\r\n", capabilityAddr, levels[i].Object, levels[i].Children);
	}
	
	uptr addr = 0x100000000;
	__fast_syscall(SYSCALL_VECTOR_MAP_INTERMEDIATE_CAPABILITY, levels[2].Object, 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);
	__fast_syscall(SYSCALL_VECTOR_MAP_INTERMEDIATE_CAPABILITY, levels[1].Object, 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);
	__fast_syscall(SYSCALL_VECTOR_MAP_INTERMEDIATE_CAPABILITY, levels[0].Object, 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);
	__fast_syscall(SYSCALL_VECTOR_MAP_CAPABILITY, frame.Object, FRAME_MEMORY, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE , 0, 0);

	mkmi_log("Trying to access page...\r\n");
	*(u32*)addr = 0xDEAD;
	mkmi_log("Result: 0x%x\r\n", *(u32*)addr);



	return 0;
}

