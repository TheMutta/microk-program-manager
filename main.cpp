#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>

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
	(void)argc, (void)argv;

	PutStr("Hello, from userland!\r\n");

	uptr capPtr = 0;
	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, 0, MEMORY_MAP_CNODE_SLOT, (usize)&capPtr, 0, 0);

	PutHex(capPtr);
	PutStr("\r\n");


	usize utCount;
	usize largestUtIndex = -1;
	usize largestUtLength = 0;
	UTHeader utPtr;
	for (utCount = 1; ;++utCount) {
		Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, capPtr, utCount, (usize)&utPtr, 0, 0);
	
		if (utPtr.Address == (~(uptr)0)) {
			break;
		}

		if (utPtr.Length >= largestUtLength) {
			largestUtLength = utPtr.Length;
			largestUtIndex = utCount;
		}
	}

	PutStr("Memory map:\r\n");
	for (usize i = 1; i < utCount; ++i) {
		Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, capPtr, i, (usize)&utPtr, 0, 0);
	
		if (utPtr.Address == (~(uptr)0)) {
			break;
		}

		PutStr("Slot: ");
		PutDec(i);
		PutStr("\r\n");

		PutStr("-> ");
		PutHex(utPtr.Address);

		PutBlank(16 - GetDigit(utPtr.Address, 16) + 1);
		PutDec(utPtr.Length / 1024);
		PutStr("kb\r\n");
	}

	const usize cnodeSize = 64 * 1024;
	usize newUtSlot;
	usize newNodeSlot;

	if (largestUtLength < cnodeSize) {
		return -1;
	}
	
	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, capPtr, largestUtIndex, cnodeSize, capPtr, (usize)&newUtSlot);
	PutStr("New slot: ");
	PutDec(newUtSlot);
	PutStr("\r\n");

	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, capPtr, newUtSlot, OBJECT_TYPE::CNODE, capPtr, (usize)&newNodeSlot);
	PutStr("New slot: ");
	PutDec(newNodeSlot);
	PutStr("\r\n");

	uptr newCapPtr;	
	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, capPtr, newNodeSlot, (usize)&newCapPtr, 0, 0);

	PutHex(newCapPtr);
	PutStr("\r\n");

	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_ADD_CNODE, capPtr, newNodeSlot, 0, 0, 0);

	PutStr("Node added to cspace\r\n");


	/*
	Syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, capPtr, newSlot, (usize)&utPtr, 0, 0);
	PutHex(utPtr.Address);

	PutBlank(16 - GetDigit(utPtr.Address, 16) + 1);
	PutDec(utPtr.Length / 1024);
	PutStr("kb\r\n");
	*/

	PutStr("\r\n");

	return 0;
}
