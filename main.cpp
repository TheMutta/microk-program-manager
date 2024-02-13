#include <stdint.h>
#include <stddef.h>
#include <mkmi_syscall.h>
#include <mkmi_capability.h>

void PutStr(const char *str) {
	Syscall(SYSCALL_VECTOR_DEBUG, (usize)str, 's', 0, 0, 0, 0);
}

void PutHex(usize hex) {
	Syscall(SYSCALL_VECTOR_DEBUG, hex, 'x', 0, 0, 0, 0);
}

void PutDec(long dec) {
	Syscall(SYSCALL_VECTOR_DEBUG, dec, 'd', 0, 0, 0, 0);
}

extern "C" int Main(void *parent) {
	PutStr("Hello, from userland!\r\n");

	uptr cnodeCapPtr = 0;
	usize cnodeSlot = 4;

	MKMI_Capability memoryCapability;

	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, 0, cnodeSlot, (usize)&cnodeCapPtr, 0, 0);
	cnodeSlot = 1;

	PutStr("Memory regions:\r\n");
	do {
		Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_CAP, cnodeCapPtr, cnodeSlot, (usize)&memoryCapability, 0, 0);
		
		PutHex(memoryCapability.Type);
		PutStr("\r\n");
		PutHex(memoryCapability.Object);
		PutStr("\r\n");
		PutHex(memoryCapability.Size);
		PutStr("\r\n");

		++cnodeSlot;
	} while(memoryCapability.Type != MKMI_ObjectType::NULL_CAPABILITY);
		
	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_CAP, cnodeCapPtr, 1, (usize)&memoryCapability, 0, 0);

	int result = 0;
	uptr addr = memoryCapability.Object;
	uptr virt = 0x40000000;

	PutStr("Mapping ");
	PutHex(addr);
	PutStr("...\r\n");

	Syscall(SYSCALL_VECTOR_ARCHCTL, SYSCALL_ARCHCTL_MAP_INTERMEDIATE, (usize)&result, 3, addr, virt, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	PutDec(result);
	PutStr("\r\n");
	if (result == 1) {
		result = 0;
	} else {
		addr += 0x1000;
	}

	Syscall(SYSCALL_VECTOR_ARCHCTL, SYSCALL_ARCHCTL_MAP_INTERMEDIATE, (usize)&result, 2, addr, virt, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	PutDec(result);
	PutStr("\r\n");
	if (result == 1) {
		result = 0;
	} else {
		addr += 0x1000;
	}

	Syscall(SYSCALL_VECTOR_ARCHCTL, SYSCALL_ARCHCTL_MAP_INTERMEDIATE, (usize)&result, 1, addr, virt, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE);
	PutDec(result);
	PutStr("\r\n");
	if (result == 1) {
		result = 0;
	} else {
		addr += 0x1000;
	}

	Syscall(SYSCALL_VECTOR_ARCHCTL, SYSCALL_ARCHCTL_MAP_PAGE, (usize)&result, addr, virt, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0);
	PutDec(result);
	PutStr("\r\n");

	*(u32*)virt = 0x69;
	PutHex(virt);
	PutStr(" -> ");
	PutHex(*(u32*)virt);
	PutStr("\r\n");

	(void)parent;
	return 0;
}
