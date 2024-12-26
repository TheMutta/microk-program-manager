#include "ramfs.hpp"
	
#include <mkmi.h>

RamFS::RamFS(VFS *vfs, MemoryMapper *mapper, Heap *kernelHeap, usize initialSize) : Vfs(vfs), Mapper(mapper), KernelHeap(kernelHeap), Size(initialSize) {
	Nodes = (RamFSNode*)kernelHeap->Malloc(sizeof(RamFSNode) * initialSize);
	memset(Nodes, 0, sizeof(RamFSNode) * Size);

	Nodes[0].NodeID = 0;
	Nodes[0].Type = VFS_NODE_DIR;
	RamFSNode *rootNode = (RamFSNode*)&Nodes[0];

	rootNode->Data.Directory.Contents = (RamFSDirContents*)kernelHeap->Malloc(sizeof(RamFSDirContents));
	rootNode->Data.Directory.Contents->Node.NodeID = 0;
	strcpy(rootNode->Data.Directory.Contents->Node.NodeName, ".");
	rootNode->Data.Directory.Contents->Node.Type = VFS_NODE_DIR;
	rootNode->Data.Directory.Contents->Previous = nullptr;

	rootNode->Data.Directory.Contents->Next = (RamFSDirContents*)kernelHeap->Malloc(sizeof(RamFSDirContents));
	rootNode->Data.Directory.Contents->Next->Node.NodeID = 0;
	strcpy(rootNode->Data.Directory.Contents->Next->Node.NodeName, "..");
	rootNode->Data.Directory.Contents->Next->Node.Type = VFS_NODE_DIR;
	rootNode->Data.Directory.Contents->Next->Previous = rootNode->Data.Directory.Contents;

	rootNode->Data.Directory.Contents->Next->Next = nullptr;
}

int RamFS::GetRoot(VFSNodeHandle *nodeHandle) {
	RamFSNode *rootNode = (RamFSNode*)&Nodes[0];
	nodeHandle->Fs = reinterpret_cast<FS*>(this);
	nodeHandle->NodeID = 0;

	return 0;
}
	
int RamFS::Open(VFSNodeHandle base, const char *name, VFSNodeHandle *nodeHandle) {

}
	
int RamFS::MkDir(VFSNodeHandle base, const char *name, VFSNodeHandle *nodeHandle) {
	if (base.NodeID >= Size) return -1;
	if (Nodes[base.NodeID].Type != VFS_NODE_DIR) return -1;

	RamFSNode *node = AllocateNode();
	node->Type = VFS_NODE_DIR;
	strcpy(node->NodeName, name);

	node->Data.Directory.Contents = (RamFSDirContents*)KernelHeap->Malloc(sizeof(RamFSDirContents));
	node->Data.Directory.Contents->Node.NodeID = node->NodeID;
	strcpy(node->Data.Directory.Contents->Node.NodeName, ".");
	node->Data.Directory.Contents->Node.Type = VFS_NODE_DIR;
	node->Data.Directory.Contents->Previous = nullptr;

	node->Data.Directory.Contents->Next = (RamFSDirContents*)KernelHeap->Malloc(sizeof(RamFSDirContents));
	node->Data.Directory.Contents->Next->Node.NodeID = node->NodeID;
	strcpy(node->Data.Directory.Contents->Next->Node.NodeName, "..");
	node->Data.Directory.Contents->Next->Node.Type = VFS_NODE_DIR;
	node->Data.Directory.Contents->Next->Previous = node->Data.Directory.Contents;

	node->Data.Directory.Contents->Next->Next = nullptr;

	RamFSNode *baseNode = &Nodes[base.NodeID];
	RamFSDirContents *dir = (RamFSDirContents*)KernelHeap->Malloc(sizeof(RamFSDirContents));
	dir->Node = *node;
	dir->Previous = baseNode->Data.Directory.Contents;
	baseNode->Data.Directory.Contents->Next->Previous = dir;
	baseNode->Data.Directory.Contents->Next = dir;

	nodeHandle->Fs = reinterpret_cast<FS*>(this);
	nodeHandle->NodeID = node->NodeID;

	return 0;
}
