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

void PutBlank(long count) {
	for (int i = 0; i < count; ++i) {
		PutStr(" ");
	}
}

int GetDigit(usize number, usize base) {
	int digit = 1;

	while(number >= base) {
		number /= base;
		++digit;
	}

	return digit;
}

struct UTHeader {
	uptr Address;
	usize Length;
	u32 Flags;
}__attribute__((packed));

extern "C" int Main(int argc, char **argv) {
	PutStr("Hello, from userland!\r\n");

	if (argc != 1) {
		return -1;
	}

	uptr *initInfo = (uptr*)argv;

	PutHex((uptr)initInfo);
	PutStr("\r\n");
	PutHex(*initInfo);
	PutStr("\r\n");

	
	uptr capPtr = 0;
	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, 0, MEMORY_MAP_CNODE_SLOT, (usize)&capPtr, 0, 0);

	PutHex(capPtr);
	PutStr("\r\n");

	PutStr("Memory map:\r\n");
	UTHeader utPtr;
	for (int i = 1; utPtr.Address != (~(uptr)0); ++i) {
		Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, capPtr, i, (usize)&utPtr, 0, 0);
	
		PutStr("-> ");
		PutHex(utPtr.Address);

		PutBlank(16 - GetDigit(utPtr.Address, 16) + 1);
		PutDec(utPtr.Length / 1024);
		PutStr("kb\r\n");
	}


	PutStr("\r\n");

	return 0;
}
