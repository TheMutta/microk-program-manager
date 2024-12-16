#pragma once

#include "memory.hpp"

class VFS {
public:
	enum VFSNodeType {
		VFS_NODE_FILE,
		VFS_NODE_DIR,
	};

	struct DataBlock {
		DataBlock *Next, *Previous;
#define BLOCK_SIZE 512
		u8 Data[BLOCK_SIZE];
	};

	struct VFSNode {
		u64 NodeID;
		VFSNodeType Type;
		DataBlock *Data;
	};

	struct DirectoryEntry {
		u64 NodeID;
		usize Length;
		VFSNodeType Type;
		u8 Name[];
	};

	VFS() = default;
	VFS(MemoryMapper *mapper, Heap *kernelHeap);

	VFSNode *ResolvePath(const char *path);
	void DebugListDirectory(VFSNode *node);
private:
	VFSNode *RootNode;

	MemoryMapper *Mapper;
	Heap *KernelHeap;
};
