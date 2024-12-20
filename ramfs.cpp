#include "ramfs.hpp"
	

RamFS::RamFS(VFS *vfs, MemoryMapper *mapper, Heap *kernelHeap, usize initialSize) : Vfs(vfs), Mapper(mapper), KernelHeap(kernelHeap) {

}

int Open(VFSNodeHandle handle) {}
usize Read(VFSNodeHandle handle, void *buffer, usize length) {}
usize Write(VFSNodeHandle handle, void *buffer, usize length) {}
int Close(VFSNodeHandle handle) {}

int OpenDir(VFSNodeHandle handle) {}
usize ReadDir(VFSNodeHandle handle, VFSDirNode *buffer) {}
int CloseDir(VFSNodeHandle handle) {}
