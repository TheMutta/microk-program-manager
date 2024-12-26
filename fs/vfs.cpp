#include "vfs.hpp"

#include <mkmi.h>

VFS::VFS(MemoryMapper *mapper, Heap *kernelHeap) : Mapper(mapper), KernelHeap(kernelHeap), MountPoints(nullptr) {

}

void VFS::DebugDump() {
	mkmi_log("Mount points:\r\n");

	VFSMount *current = MountPoints;
	while(current) {
		mkmi_log(" 0x%x-%d -> 0x%x-%d\r\n",
				current->MountPoint.Fs,
				current->MountPoint.NodeID,
				current->MountedFS.Fs,
				current->MountedFS.NodeID);
		current = current->Next;
	}
}


void VFS::Mount(VFSNodeHandle mountPoint, VFSNodeHandle mountedFS) {
	if (MountPoints == nullptr) {
		mkmi_log("Mounting VFS root.\r\n");
		MountPoints = (VFSMount*)KernelHeap->Malloc(sizeof(VFSMount));
		MountPoints->MountPoint = mountPoint;
		MountPoints->MountedFS = mountedFS;
		MountPoints->Next = MountPoints->Previous = nullptr;
	} else {
		mkmi_log("Mounting in VFS.\r\n");
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
	char *name = strtok(path + 1, "/", &savePtr);
	mkmi_log("Name: %s\r\n", name);

	while (name) {
		name = strtok(nullptr, "/", &savePtr);

		if (name != nullptr) {
			mkmi_log("Name: %s\r\n", name);
		}
	}

	return 0;
}

