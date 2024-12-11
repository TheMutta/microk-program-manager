#include <mkmi.h>
#include "memory.hpp"

Heap::Heap(uptr address, usize initialSize) : Address(address), Size(initialSize) {
	memset(Address, 0, initialSize);

	RootBlock = (HeapBlock*)Address;
	RootBlock->Next = RootBlock->Previous = NULL;
	RootBlock->Size = Size - sizeof(HeapBlock);
	RootBlock->IsFree = true;
	

	mkmi_log("HeapSize: %d\r\n", RootBlock->Size);

}

void *Heap::Malloc(usize size) {
	mkmi_log("Called Malloc for size: %d\r\n", size);

	if (size < 128) {
		ROUND_UP_TO(size, 128);
	}

	for(HeapBlock *block = RootBlock; block; block = block->Next) {
		if (!block->IsFree) continue;

		if (block->Size == size) {
			block->IsFree = false;
			return (void*)((uptr)block + sizeof(HeapBlock));
		} else if (block->Size > size) {
			// SPLIT
			HeapBlock *newBlock = (HeapBlock*)((uptr)block + sizeof(HeapBlock) + size);
			newBlock->Size = block->Size - size - sizeof(HeapBlock);
			newBlock->IsFree = true;
			newBlock->Previous = block;
			newBlock->Next = block->Next;

			if (block->Next) {
				block->Next->Previous = newBlock;
			}

			block->Next = newBlock;
			block->Size = size;
			block->IsFree = false;
			return (void*)((uptr)block + sizeof(HeapBlock));
		} else {
			// TOO SMALL
			continue;
		}
	}

	mkmi_log("OUT OF MEMORY\r\n");
	DebugDump();

	return NULL;
}

void Heap::Free(void *ptr) {
	HeapBlock *block = (HeapBlock*)((uptr)ptr - sizeof(HeapBlock));
	block->IsFree = true;
}
	
void Heap::ExpandHeap(usize amount) {

}

void Heap::DebugDump() {
	for(HeapBlock *block = RootBlock; block; block = block->Next) {
		mkmi_log(" %s block 0x%x of size %d\r\n",
			block->IsFree ? "Free" :"Used", block, block->Size);
	}
}
