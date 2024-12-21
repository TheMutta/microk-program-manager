#include "vfs.hpp"

#include <mkmi.h>

VFS::VFS(MemoryMapper *mapper, Heap *kernelHeap) : Mapper(mapper), KernelHeap(kernelHeap), MountPoints(nullptr) {

}

void VFS::Mount(VFSNodeHandle mountPoint, VFSNodeHandle mountedFS) {
	if (MountPoints == nullptr) {
		MountPoints = (VFSMount*)KernelHeap->Malloc(sizeof(VFSMount));
		MountPoints->MountPoint = mountPoint;
		MountPoints->MountedFS = mountedFS;
		MountPoints->Next = MountPoints->Previous = nullptr;
	} else {
		VFSMount *newMount = (VFSMount*)KernelHeap->Malloc(sizeof(VFSMount));
		newMount->MountPoint = mountPoint;
		newMount->MountedFS = mountedFS;

		newMount->Next = MountPoints->Next;

		if(MountPoints->Next) {
			MountPoints->Next->Previous = newMount;
		}
		
		newMount->Previous = MountPoints;
		MountPoints->Next = newMount;
	}
}
	
int VFS::ResolvePath(char *path, VFSNodeHandle *nodeHandle) {
	nodeHandle->NodeID = -1;
	nodeHandle->Fs = nullptr;

	if (MountPoints == nullptr) return -1;

	VFSMount *currentMount = MountPoints;

	if (strcmp(path, "/") == 0) {
		nodeHandle->NodeID = currentMount->MountedFS.NodeID;
		nodeHandle->Fs = currentMount->MountedFS.Fs;
	}

	char *savePtr;
	char *name = strtok(path, "/", &savePtr);
	mkmi_log("Name: 0x%x", name);

	while (name) {
		name = strtok(nullptr, "/", &savePtr);
		mkmi_log("Name: 0x%x", name);
	}

	return 0;
}

