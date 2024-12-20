#pragma once

#include "memory.hpp"

class VFS;
class FS;

enum VFSNodeType {
	VFS_NODE_FILE,
	VFS_NODE_DIR,
};

enum VFSNodeMode {
	VFS_NODE_RDONLY,
	VFS_NODE_RDWR,
	VFS_NODE_WRONLY
};

struct VFSNode {
	u64 NodeID;
	VFSNodeType Type;
};

struct VFSNodeHandle {
	FS *Fs;
	u64 NodeID;
};

struct VFSDirNode {
	u64 NodeID;
	char NodeName[256];
};

class VFS {
public:
	VFS() = default;
	VFS(MemoryMapper *mapper, Heap *kernelHeap);

	int Open(VFSNodeHandle handle);
	usize Read(VFSNodeHandle handle, void *buffer, usize length);
	usize Write(VFSNodeHandle handle, void *buffer, usize length);
	int Close(VFSNodeHandle handle);
private:
	MemoryMapper *Mapper;
	Heap *KernelHeap;
};
