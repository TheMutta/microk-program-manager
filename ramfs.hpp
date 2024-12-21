#pragma once

#include "vfs.hpp"

class RamFS : public FS {
public:
	RamFS(VFS *vfs, MemoryMapper *mapper, Heap *kernelHeap, usize initialSize);
	~RamFS() { }

	
	int Open(u64 node, VFSNodeHandle *nodeHandle) override;
	int MkDir(VFSNodeHandle base, const char *name, VFSNodeHandle *nodeHandle) override;
private:
	struct RamFSDirContents;

	struct RamFSNode : public VFSNode {
		union {
			struct {
				u8 *Data;
				usize DataSize;
			} File;
			struct {
				RamFSDirContents *Contents;
			} Directory;
		} Data;

	};

	struct RamFSDirContents {
		RamFSNode Node;
		RamFSDirContents *Next, *Previous;
	};
	

	RamFSNode *AllocateNode() {
		static u64 id = 0;
		++id;
		Nodes[id].NodeID = id;
		return &Nodes[id];
	}
	
	VFS *Vfs;
	MemoryMapper *Mapper;
	Heap *KernelHeap;

	RamFSNode *Nodes;
	usize Size;
};
