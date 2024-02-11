#include <stdint.h>
#include <stddef.h>
#include <mkmi_syscall.h>
#include <mkmi_capability.h>

void PutStr(const char *str) {
	Syscall(0, (size_t)str, 's', 0, 0, 0, 0);
}

void PutHex(size_t hex) {
	Syscall(0, hex, 'x', 0, 0, 0, 0);
}

extern "C" int Main(void *parent) {
	PutStr("Hello, from userland!\r\n");

	uintptr_t cnodePtr = 0;
	size_t cnodeSlot = 4;

	MKMI_Capability cnodeCapability;
	MKMI_Capability memoryCapability;

	Syscall(2, cnodePtr, cnodeSlot, (size_t)&cnodeCapability, 0, 0, 0);
	cnodeSlot = 1;

	PutStr("Memory regions:\r\n");
	do {
		Syscall(2, cnodeCapability.Object, cnodeSlot, (size_t)&memoryCapability, 0, 0, 0);
		
		PutHex(memoryCapability.Type);
		PutStr("\r\n");
		PutHex(memoryCapability.Object);
		PutStr("\r\n");
		PutHex(memoryCapability.Size);
		PutStr("\r\n");

		++cnodeSlot;
	} while(memoryCapability.Type != MKMI_ObjectType::NULL_CAPABILITY);

	(void)parent;
	return 0;
}
