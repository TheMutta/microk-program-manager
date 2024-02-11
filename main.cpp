#include <stdint.h>
#include <stddef.h>
#include <mkmi_syscall.h>

void PutStr(const char *str) {
	Syscall(0, (size_t)str, 's', 0, 0, 0, 0);
}

void PutHex(size_t hex) {
	Syscall(0, hex, 'x', 0, 0, 0, 0);
}

extern "C" int Main(void *parent) {
	PutStr("Hello, from userland!\r\n");

	uintptr_t cnodePtr = 0;
	uintptr_t memoryRegion = 0;
	size_t cnodeSlot = 4;
	size_t capSize = 0;
	Syscall(2, cnodePtr, cnodeSlot, (size_t)&cnodePtr, (size_t)&capSize, 0, 0);
	cnodeSlot = 1;

	PutStr("Memory regions:\r\n");
	while (capSize != 0) {
		Syscall(2, cnodePtr, cnodeSlot, (size_t)&memoryRegion, (size_t)&capSize, 0, 0);
		
		PutHex(memoryRegion);
		PutStr("\r\n");
		PutHex(capSize);
		PutStr("\r\n");

		++cnodeSlot;
	}

	(void)parent;
	return 0;
}
