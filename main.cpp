#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>

struct UTHeader {
	uptr Address;
	usize Length;
	u32 Flags;
}__attribute__((packed));

usize GetCapabilityPointer(uptr node, usize slot) {
	uptr pointer = 0;
	Syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, node, slot, (usize)&pointer);
	return pointer;
}

void GetUTHeader(uptr node, usize slot, UTHeader *header) {
	Syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, node, slot, (usize)header);
}

extern "C" int Main(int argc, char **argv) {
	(void)argc, (void)argv;
	uptr capPtr = GetCapabilityPointer(0, MEMORY_MAP_CNODE_SLOT);

	MKMI_Log("0x%x\r\n", capPtr);

	usize utCount;
	usize largestUtIndex = -1;
	usize largestUtLength = 0;
	UTHeader utPtr;
	for (utCount = 1; ;++utCount) {
		GetUTHeader(capPtr, utCount, &utPtr);
	
		if (utPtr.Address == (~(uptr)0)) {
			break;
		}

		if (utPtr.Length >= largestUtLength) {
			largestUtLength = utPtr.Length;
			largestUtIndex = utCount;
		}
	}

	MKMI_Log("Memory map:\r\n");
	for (usize i = 1; i < utCount; ++i) {
		GetUTHeader(capPtr, i, &utPtr);
	
		if (utPtr.Address == (~(uptr)0)) {
			break;
		}

		MKMI_Log("Slot: %d\r\n", i);
		MKMI_Log("-> 0x%x %dkb\r\n", utPtr.Address, utPtr.Length / 1024);
	}

	const usize cnodeSize = 64 * 1024;
	usize newUtSlot;
	usize newNodeSlot;

	if (largestUtLength < cnodeSize) {
		return -1;
	}
	
	Syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, capPtr, largestUtIndex, cnodeSize, capPtr, (usize)&newUtSlot, 2);
	MKMI_Log("New slot: %d\r\n", newUtSlot);

	Syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, capPtr, newUtSlot, OBJECT_TYPE::CNODE, capPtr, (usize)&newNodeSlot, CAPABILITY_RIGHTS::ACCESS);
	MKMI_Log("New slot: %d\r\n", newNodeSlot);

	uptr newCapPtr;	
	Syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, capPtr, newNodeSlot, (usize)&newCapPtr);
	MKMI_Log("0x%x\r\n", newCapPtr);

	Syscall(4, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_ADD_CNODE, capPtr, newNodeSlot);

	MKMI_Log("Node added to cspace\r\n");

	usize utFrameSlot;
	usize frameSlot;
	Syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, capPtr, newUtSlot + 1, 4096, newCapPtr, (usize)&utFrameSlot, 2);
	Syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, newCapPtr, utFrameSlot, OBJECT_TYPE::FRAMES, newCapPtr, (usize)&frameSlot, CAPABILITY_RIGHTS::ACCESS);

	/*
	__syscall(SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, capPtr, newSlot, (usize)&utPtr, 0, 0);
	PutHex(utPtr.Address);

	PutBlank(16 - GetDigit(utPtr.Address, 16) + 1);
	PutDec(utPtr.Length / 1024);
	PutStr("kb\r\n");
	*/
	
	Syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, 0);
	Syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, capPtr);
	Syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, newCapPtr);

	return 0;
}
