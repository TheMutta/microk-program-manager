#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>

struct ut_header {
	uptr address;
	usize length;
	u32 flags;
}__attribute__((packed));

struct heap_header {
	heap_header *previous, *next;
	usize size;
	bool used;
}__attribute__((packed));

heap_header *firstHeader;

void *malloc(usize size);
void free(void *ptr);
void dump_hdrs();

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

	uptr initrdAddr = __mkmi_get_arg_index(0);
	usize initrdSize = __mkmi_get_arg_index(1);
	
	mkmi_log("Initrd: [0x%x - 0x%x]\r\n", initrdAddr, initrdAddr + initrdSize);


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

	const usize cnode_size = 1 * 1024 * 1024;
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
	const usize frame_count = 512;
	usize frame_slot[frame_count];
	ut_header new_hdr;

	get_ut_header(cap_ptr, new_ut_slot + 1, &new_hdr);
	mkmi_log("Region size: %d frames\r\n", new_hdr.length / 4096);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_SPLIT, cap_ptr, new_ut_slot + 1, 4096, new_cap_ptr, (usize)&ut_frame_slot, 3 + frame_count + 1);

	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl3_slot, CAPABILITY_RIGHTS::ACCESS, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot + 1, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl2_slot, CAPABILITY_RIGHTS::ACCESS, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot + 2, OBJECT_TYPE::PAGING_STRUCTURE, new_cap_ptr, (usize)&lvl1_slot, CAPABILITY_RIGHTS::ACCESS, 0);

	for (usize i = 0; i < frame_count; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_RETYPE, new_cap_ptr, ut_frame_slot + 3 + i, OBJECT_TYPE::FRAMES, new_cap_ptr, (usize)&frame_slot[i], CAPABILITY_RIGHTS::ACCESS, 0);
	}

	uptr addr = 0xDEAD000000;
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl3_slot, 3, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl2_slot, 2, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_INTERMEDIATE, new_cap_ptr, lvl1_slot, 1, addr, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);

	for (usize i = 0; i < frame_count; ++i) {
		syscall(8, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_MAP_PAGE, new_cap_ptr, frame_slot[i], addr + 4096 * i, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, 0, 0);
	}

	for (usize i = 0; i < frame_count * 4096; ++i) {
		*((volatile u8*)(addr + i)) = 0x00;
	}

	heap_header *hdr = (heap_header*)addr;
	hdr->previous = hdr->next = NULL;
	hdr->used = false;
	hdr->size = frame_count * 4096 - sizeof(heap_header);
	firstHeader = hdr;
	
	mkmi_log("Heap at 0x%x of size %d bytes\r\n", addr, frame_count * 4096);

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

	while(true) { }

	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, 0);
	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, cap_ptr);
	syscall(3, SYSCALL_VECTOR_CAPCTL, SYSCALL_CAPCTL_DEBUG, new_cap_ptr);
	
	return 0;
}

void dump_hdrs() {
	mkmi_log("HEAP_START\r\n");
	for (heap_header *hdr = firstHeader; hdr != NULL; hdr = hdr->next) {
		if (!hdr) {
			break;
		}

		mkmi_log("HEAP_HDR 0x%x, %d bytes, %s\r\n", (void*)hdr, hdr->size, hdr->used ? "used" : "unused");
	}
	mkmi_log("HEAP_END\r\n");
}

void *malloc(usize size) {
	for (heap_header *hdr = firstHeader; hdr != NULL; hdr = hdr->next) {
		if (!hdr) break;
		if (hdr->used) continue;
		if (hdr->size < size) continue;
		
		if (hdr->size == size) {
			hdr->used = true;
			return (void*)((uptr)hdr + sizeof(heap_header));
		} else if (hdr->size > size) {
			if (hdr->size > size + sizeof(heap_header)) {
				heap_header *next = (heap_header*)((uptr)hdr + sizeof(heap_header) + size);
				next->used = false;
				next->size = hdr->size - size - sizeof(heap_header);
				hdr->size -= next->size + sizeof(heap_header);

				next->previous = hdr;
				next->next = hdr->next;

				if (hdr->next) {
				    hdr->next->previous = next;
				}

				hdr->next = next;

				hdr->used = true;
				return (void*)((uptr)hdr + sizeof(heap_header));
			} else {
				hdr->used = true;
				return (void*)((uptr)hdr + sizeof(heap_header));
			}
		} else {
			continue;
		}
	}

	return NULL;
}

void free(void *ptr) {
	heap_header *hdr = (heap_header*)((uptr)ptr - sizeof(heap_header));

	hdr->used = false;

	bool cleanup = false;
	while (true) {
		if (hdr->next && !hdr->next->used) {
			cleanup = true;

			heap_header *nextHdr = hdr->next;
			if (nextHdr->next) {
				nextHdr->previous = hdr;
			}

			hdr->size += nextHdr->size + sizeof(heap_header);
			hdr->next = nextHdr->next;
		}

		if (hdr->previous && !hdr->previous->used) {
			cleanup = true;

			heap_header *prevHdr = hdr->previous;

			if(hdr->next) {
				hdr->next->previous = prevHdr;
			}

			prevHdr->next = hdr->next;
			prevHdr->size += hdr->size + sizeof(heap_header);

			hdr = prevHdr;
		}

		if (cleanup) {
			cleanup = false;
		} else {
			break;
		}
	}
}
