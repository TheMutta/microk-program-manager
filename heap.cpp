#include <mkmi.h>
#include "memory.hpp"

Heap::Heap(uptr address, usize initialSize) : Address(address), Size(initialSize) {
	RootBlock = (HeapBlock*)Address;
	RootBlock->Next = RootBlock->Previous = NULL;
	RootBlock->Size = Size - sizeof(HeapBlock);
	RootBlock->IsFree = true;
}

void *Heap::Malloc(usize size) {
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

			block->IsFree = false;
			return (void*)((uptr)block + sizeof(HeapBlock));
		} else {
			// TOO SMALL
			continue;
		}
	}

	return NULL;
}

void Heap::Free(void *ptr) {
	HeapBlock *block = (HeapBlock*)((uptr)ptr - sizeof(HeapBlock));
	block->IsFree = true;
}
	
void Heap::ExpandHeap(usize amount) {

}
