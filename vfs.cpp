#include "vfs.hpp"

#include <mkmi.h>

VFS::VFS(MemoryMapper *mapper, Heap *kernelHeap) : Mapper(mapper), KernelHeap(kernelHeap) {

}
	

int VFS::Open(VFSNodeHandle handle) {
}

usize VFS::Read(VFSNodeHandle handle, void *buffer, usize length) {
}

usize VFS::Write(VFSNodeHandle handle, void *buffer, usize length) {
}
	
int VFS::Close(VFSNodeHandle handle) {
}
