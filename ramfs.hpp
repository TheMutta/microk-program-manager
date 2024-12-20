#pragma once

#include "vfs.hpp"

class RamFS {
public:
	RamFS(VFS *vfs, MemoryMapper *mapper, Heap *kernelHeap, usize initialSize);
	~RamFS() { }

	int Open(VFSNodeHandle handle);
	usize Read(VFSNodeHandle handle, void *buffer, usize length);
	usize Write(VFSNodeHandle handle, void *buffer, usize length);
	int Close(VFSNodeHandle handle);

	int OpenDir(VFSNodeHandle handle);
	usize ReadDir(VFSNodeHandle handle, VFSDirNode *buffer);
	int CloseDir(VFSNodeHandle handle);
private:
	VFS *Vfs;
	MemoryMapper *Mapper;
	Heap *KernelHeap;

};
