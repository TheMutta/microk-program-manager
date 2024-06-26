#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>

struct ut_header {
	uptr address;
	usize length;
	u32 flags;
}__attribute__((packed));

usize get_capability_pointer(uptr node, usize slot) {
	uptr pointer = 0;
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, node, slot, (usize)&pointer);
	return pointer;
}

void get_ut_header(uptr node, usize slot, ut_header *header) {
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, node, slot, (usize)header);
}

extern "C" int Main(int argc, char **argv) {
	(void)argc, (void)argv;
	uptr cap_ptr = get_capability_pointer(0, MEMORY_MAP_CNODE_SLOT);

	mkmi_log("0x%x\r\n", cap_ptr);

	usize ut_count;
	usize largest_ut_index = -1;
	usize largest_ut_length = 0;
	ut_header ut_ptr;

	for (ut_count = 1; ;++ut_count) {
		get_ut_header(cap_ptr, ut_count, &ut_ptr);
	
		if (ut_ptr.address == (~(uptr)0)) {
			break;
		}

		if (ut_ptr.length >= largest_ut_length) {
			largest_ut_length = ut_ptr.length;
			largest_ut_index = ut_count;
		}
	}

	mkmi_log("Memory map:\r\n");
	for (usize i = 1; i < ut_count; ++i) {
		get_ut_header(cap_ptr, i, &ut_ptr);
	
		if (ut_ptr.address == (~(uptr)0)) {
			break;
		}

		mkmi_log("Slot: %d\r\n", i);
		mkmi_log("-> 0x%x %dkb\r\n", ut_ptr.address, ut_ptr.length / 1024);
	}

	const usize cnode_size = 64 * 1024;
	usize new_ut_slot;
	usize new_node_slot;

	if (largest_ut_length < cnode_size) {
		return -1;
	}
	
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, cap_ptr, largest_ut_index, cnode_size, cap_ptr, (usize)&new_ut_slot, 2);
	mkmi_log("New slot: %d\r\n", new_ut_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, cap_ptr, new_ut_slot, OBJECT_TYPE::CNODE, cap_ptr, (usize)&new_node_slot, CAPABILITY_RIGHTS::ACCESS);
	mkmi_log("New slot: %d\r\n", new_node_slot);

	uptr new_cap_ptr;	
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, cap_ptr, new_node_slot, (usize)&new_cap_ptr);
	mkmi_log("0x%x\r\n", new_cap_ptr);

	syscall(4, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_ADD_CNODE, cap_ptr, new_node_slot);

	mkmi_log("Node added to cspace\r\n");

	usize ut_frame_slot;
	usize lvl3_slot;
	usize lvl2_slot;
	usize lvl1_slot;
	const usize frame_count = 10;
	usize frame_slot[frame_count];
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, cap_ptr, new_ut_slot + 1, 4096, new_cap_ptr, (usize)&ut_frame_slot, 3 + frame_count + 1);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl3_slot, CAPABILITY_RIGHTS::ACCESS, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot + 1, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl2_slot, CAPABILITY_RIGHTS::ACCESS, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot + 2, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl1_slot, CAPABILITY_RIGHTS::ACCESS, 0);

	for (usize i = 0; i < frame_count; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot + 3 + i, OBJECT_TYPE::FRAMES, new_cap_ptr, (usize)&frame_slot[i], CAPABILITY_RIGHTS::ACCESS, 0);
	}

	uptr addr = 0xDEAD0000;
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl3_slot, 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl2_slot, 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl1_slot, 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);

	for (usize i = 0; i < frame_count; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_PAGE, new_cap_ptr, frame_slot[i], addr + 4096 * i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	}

	*((u32*)addr) = 0x0BADCACA;
	mkmi_log("Value: 0x%x\r\n", *((u32*)addr));

/*
	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, new_cap_ptr);
*/
	return 0;
}
