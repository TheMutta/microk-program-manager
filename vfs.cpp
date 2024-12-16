#include "vfs.hpp"

#include <mkmi.h>

VFS::VFS(MemoryMapper *mapper, Heap *kernelHeap) : Mapper(mapper), KernelHeap(kernelHeap) {
	RootNode = (VFSNode*)kernelHeap->Malloc(sizeof(VFSNode));
	RootNode->NodeID = 1;
	RootNode->Type = VFS_NODE_DIR;
	RootNode->Data = (DataBlock*)kernelHeap->Malloc(sizeof(DataBlock));
	RootNode->Data->Next = RootNode->Data->Previous = NULL;
	DirectoryEntry *entries = (DirectoryEntry*)RootNode->Data->Data;
	entries[0].NodeID = 1;
	entries[0].Length = sizeof(DirectoryEntry) + 2;
	entries[0].Type= VFS_NODE_DIR;
	memcpy(&entries[0].Name, ".", 2);
	entries[1].NodeID = 1;
	entries[1].Length = sizeof(DirectoryEntry) + 3;
	entries[1].Type= VFS_NODE_DIR;
	memcpy(&entries[1].Name, "..", 3);
}
	
VFS::VFSNode *VFS::ResolvePath(const char *path) {
	if (strcmp(path, "/") == 0) {
		return RootNode;
	}

	return NULL;
}

void VFS::DebugListDirectory(VFSNode *node) {
	if (!node || node->Type != VFS_NODE_DIR) return;

	DataBlock *block = node->Data;
	DirectoryEntry *entries = (DirectoryEntry*)block->Data;
	usize maxEntriesInBlock = BLOCK_SIZE / sizeof(DirectoryEntry);
	mkmi_log("Listing with %d entries per block\r\n", maxEntriesInBlock);

	bool done = false;
	while(!done) {
		for (usize i = 0; i < maxEntriesInBlock; ++i) {
			if(entries[i].NodeID == 0) { done = true; break; }

			mkmi_log("NodeID: %d Name: %s\r\n", entries[i].NodeID, entries[i].Name);
		}

		block = block->Next;
		if (block == NULL) break;

		entries = (DirectoryEntry*)block->Data;
	}

}
