#pragma once

#include "memory.hpp"

class VFS;
class FS;

enum VFSNodeType {
	VFS_NODE_FILE = 1,
	VFS_NODE_DIR,
};

enum VFSNodeMode {
	VFS_NODE_RDONLY,
	VFS_NODE_RDWR,
	VFS_NODE_WRONLY
};

struct VFSNode {
	u64 NodeID;
	char NodeName[256];
	VFSNodeType Type;
};

struct VFSNodeHandle {
	FS *Fs;
	u64 NodeID;
};

struct VFSDirNode {
	u64 NodeID;
};

class FS {
public:
	virtual int Open(u64 node, VFSNodeHandle *nodeHandle) = 0;
	virtual int MkDir(VFSNodeHandle base, const char *name, VFSNodeHandle *nodeHandle) = 0;
};

class VFS {
public:
	VFS() = default;
	VFS(MemoryMapper *mapper, Heap *kernelHeap);

	int ResolvePath(char *path, VFSNodeHandle *nodeHandle);
	void Mount(VFSNodeHandle mountPoint, VFSNodeHandle mountedFS);
private:
	struct VFSMount {
		VFSNodeHandle MountPoint;
		VFSNodeHandle MountedFS;
		VFSMount *Next, *Previous;
	};

	VFSMount *MountPoints;

	MemoryMapper *Mapper;
	Heap *KernelHeap;
};
