#include "memory.hpp"

Heap::Heap(uptr address, usize initialSize) : Address(address), Size(initialSize) {
	RootBlock = (HeapBlock*)Address;
	RootBlock->Next = RootBlock->Previous = NULL;
	RootBlock->Size = Size - sizeof(HeapBlock);
	RootBlock->IsFree = true;
}

void *Heap::Malloc(usize size) {
	HeapBlock *block = RootBlock;
	while(block) {
		if (!block->IsFree) continue;

		if (block->Size == size) {
			block->IsFree = false;
			return &block->DataStart;
		} else if (block->Size > size) {
			// SPLIT
			HeapBlock *newBlock = (HeapBlock*)((uptr)&block->DataStart + size);
			newBlock->Size = block->Size - size - sizeof(HeapBlock) + sizeof(u8);
			newBlock->IsFree = true;
			newBlock->Previous = block;
			newBlock->Next = block->Next;

			if (block->Next) {
				block->Next->Previous = newBlock;
			}

			block->Next = newBlock;


			block->IsFree = false;
			return &block->DataStart;
		} else {
			// TOO SMALL
			continue;
		}

		block = block->Next;
	}

	return NULL;
}

void Heap::Free(void *ptr) {

}
	
void Heap::ExpandHeap(usize amount) {

}
