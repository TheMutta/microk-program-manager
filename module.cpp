#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include "initrd.hpp"
#include "heap.hpp"

struct ut_header {
	uptr address;
	usize length;
	u32 flags;
}__attribute__((packed));

uptr get_capability_pointer(uptr node, usize slot) {
	uptr pointer = 0;
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, node, slot, (usize)&pointer);
	return pointer;
}

void get_ut_header(uptr node, usize slot, ut_header *header) {
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_UT, node, slot, (usize)header);
}

void get_cap_obj(uptr node, usize slot, uptr *obj, u8 *type) {
	syscall(6, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_OBJ, node, slot, (usize)obj, (usize)type);
}

extern "C" int Main(int argc, char **argv) {
	(void)argc, (void)argv;

	usize total_memmap = __mkmi_get_arg_index(0);
	usize initrd_start = __mkmi_get_arg_index(1);
	usize initrd_end = __mkmi_get_arg_index(2);
	
	mkmi_log("Initrd: [0x%x - 0x%x]\r\n", initrd_start, initrd_end);

	uptr cap_ptr = get_capability_pointer(0, MEMORY_MAP_CNODE_SLOT);
				

	mkmi_log("0x%x\r\n", cap_ptr);

	usize largest_ut_index = -1;
	usize largest_ut_length = 0;

	usize initrd_start_slot = -1;
	usize initrd_end_slot = -1;

	ut_header ut_ptr;

	mkmi_log("Memory map, %d entries:\r\n", total_memmap);
	for (usize i = 1; i < total_memmap; ++i) {
		get_ut_header(cap_ptr, i, &ut_ptr);
	
		if (ut_ptr.address == (~(uptr)0)) {
			uptr obj;
			u8 type;
			get_cap_obj(cap_ptr, i, &obj, &type);

			if (type == OBJECT_TYPE::FRAMES) {
				if (obj == initrd_start) {
					initrd_start_slot = i;
				} else if (obj == initrd_end - 4096) {
					initrd_end_slot = i;

					mkmi_log("-> 0x%x %dkb Initrd\r\n", initrd_start, (initrd_end - initrd_start)/ 1024);
				} else {
					mkmi_log("-> 0x%x 4kb Frame\r\n", obj);
				}
			} else {
			}
				
			continue;
		} else {
			mkmi_log("-> 0x%x %dkb\r\n", ut_ptr.address, ut_ptr.length / 1024);

			if (ut_ptr.length >= largest_ut_length) {
				largest_ut_length = ut_ptr.length;
				largest_ut_index = i;
	
			}
		}
	}

	const usize cnode_size = 1 * 1024 * 1024;
	usize new_ut_slot;
	usize new_node_slot;

	if (largest_ut_length < cnode_size) {
		return -1;
	}
	
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, cap_ptr, largest_ut_index, cnode_size, cap_ptr, (usize)&new_ut_slot, 2);

	mkmi_log("New slot: %d\r\n", new_ut_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, cap_ptr, new_ut_slot, OBJECT_TYPE::CNODE, cap_ptr, (usize)&new_node_slot, 0xFFFF);//CAPABILITY_RIGHTS::ACCESS);

	mkmi_log("New slot: %d\r\n", new_node_slot);

	uptr new_cap_ptr;	
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_PTR, cap_ptr, new_node_slot, (usize)&new_cap_ptr);
	mkmi_log("0x%x\r\n", new_cap_ptr);

	syscall(4, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_ADD_CNODE, cap_ptr, new_node_slot);

	mkmi_log("Node added to cspace\r\n");

	usize next_region;
	usize ut_frame_slot;
	usize lvl3_slot;
	usize lvl2_slot;
	usize lvl1_slot;

	usize lvl3_initrd_slot;
	usize lvl2_initrd_slot;
	usize lvl1_initrd_slot;

	const usize frame_count = 512;
	usize frame_slot[frame_count];
	ut_header new_hdr;

	get_ut_header(cap_ptr, new_ut_slot + 1, &new_hdr);
	mkmi_log("Region size: %d frames\r\n", new_hdr.length / 4096);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, cap_ptr, new_ut_slot, &next_region);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, cap_ptr, next_region, 4096, new_cap_ptr, (usize)&ut_frame_slot, 3 + 3 + frame_count + 1);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl3_slot, 0xFFFF, 0);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl2_slot, 0xFFFF, 0);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl1_slot, 0xFFFF, 0);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl3_initrd_slot, 0xFFFF, 0);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl2_initrd_slot, 0xFFFF, 0);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl1_initrd_slot, 0xFFFF, 0);
	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);

	for (usize i = 0; i < frame_count; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::FRAMES, new_cap_ptr, (usize)&frame_slot[i], 0xFFFF, 0);
		syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, new_cap_ptr, ut_frame_slot, &ut_frame_slot);
	}

	uptr addr = 0xDEAD000000;
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl3_slot, 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl2_slot, 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl1_slot, 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);

	syscall(5, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_GET_NEXT_SLOT, cap_ptr, new_ut_slot, &next_region);

	for (usize i = 0; i < frame_count; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_PAGE, new_cap_ptr, frame_slot[i], addr + 4096 * i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	}

	for (usize i = 0; i < frame_count * 4096; ++i) {
		*((volatile u8*)(addr + i)) = 0x00;
	}

	init_heap((void*)addr, frame_count * 4096);

	void *memareas[8];
	dump_hdrs();
	memareas[0] = malloc(128);
	dump_hdrs();
	memareas[1] = malloc(256);
	dump_hdrs();
	memareas[2] = malloc(512);
	dump_hdrs();
	memareas[3] = malloc(1024);
	dump_hdrs();
	memareas[4] = malloc(2048);
	dump_hdrs();
	memareas[5] = malloc(4096);
	dump_hdrs();
	memareas[6] = malloc(8192);
	dump_hdrs();
	memareas[7] = malloc(16384);
	dump_hdrs();



	free(memareas[0]);
	dump_hdrs();
	free(memareas[2]);
	dump_hdrs();
	free(memareas[4]);
	dump_hdrs();
	free(memareas[6]);
	dump_hdrs();
	free(memareas[1]);
	dump_hdrs();
	free(memareas[3]);
	dump_hdrs();
	free(memareas[5]);
	dump_hdrs();
	free(memareas[7]);
	dump_hdrs();

	uptr initrd_addr = 0xDEAD0000000;
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl3_initrd_slot, 3, initrd_addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl2_initrd_slot, 2, initrd_addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl1_initrd_slot, 1, initrd_addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);

	mkmi_log("Initrd: [%d -  %d]\r\n", initrd_start_slot, initrd_end_slot);
	for (usize i = initrd_start_slot; i < initrd_end_slot; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_PAGE, cap_ptr, i, initrd_addr + 4096 * (i - initrd_start_slot), PAGE_PROTECTION_READ, 0, 0);
	}

	InitrdInstance instance((u8*)initrd_addr, initrd_end - initrd_start, INITRD_TAR_UNCOMPRESSED);

	while(true) { }

	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, 0);
	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, cap_ptr);
	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, new_cap_ptr);
	
	return 0;
}

