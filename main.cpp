#include <cpuid.h>
#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include <cdefs.h>
#include <object.hpp>

#include "capability.hpp"
#include "acpi.hpp"
#include "memory.hpp"
#include "serial.hpp"
#include "vfs.hpp"
#include "ramfs.hpp"

void ExceptionHandler(usize excp, usize errinfo1, usize errinfo2) {
	mkmi_log("Exception!\r\n");
	mkmi_log("%d -> 0x%x 0x%x\r\n", excp, errinfo1, errinfo2);

	while (true) { }
}

void InterruptHandler() {
	mkmi_log("Interrupt!\r\n");

	while (true) { }
}

void SyscallHandler() {
	mkmi_log("Syscall!\r\n");

	while (true) { }

}

__attribute__((used, section(".microkosm_bindings")))
static volatile ContainerBindings bindings = {
	.ExceptionHandler = ExceptionHandler,
	.InterruptHandler = InterruptHandler,
	.SyscallHandler = SyscallHandler,
};

void GetVendor() {
	u32 vendor[4] = {};
	u32 null = 0;
	__cpuid(0x40000000, null, vendor[0], vendor[1], vendor[2]);
	mkmi_log("Vendor hypervisor: 0x%x, %s\r\n", null, vendor);
}

MemoryMapper heapMapper;
MemoryMapper memoryMapper;
Heap kernelHeap;
VFS vfs;

extern "C" int Main(ContainerInfo *info) {
	mkmi_log("Hello from the containerized OS.\r\n");
	mkmi_log("Response 0x%x\r\n", info);
	mkmi_log("Initrd [0x%x-0x%x]\r\n", info->InitrdAddress, info->InitrdAddress + info->InitrdSize);
	mkmi_log("RSDP: 0x%x\r\n", info->x86_64.RSDPCapability);
	mkmi_log("DTB: 0x%x\r\n", info->DTB);

	GetVendor();

	Capability untyped;
	usize count;
	for (count = 0; ; ++count) {
		__fast_syscall(SYSCALL_VECTOR_GET_UNTYPED_CAPABILITY, (uptr)&untyped, count, 0, 0, 0, 0);
		if (untyped.Type != UNTYPED_FRAMES) break;
	}

	Capability untypedArray[count];
	bool usable[count];
	for (usize i = 0; i < count; ++i) {
		__fast_syscall(SYSCALL_VECTOR_GET_UNTYPED_CAPABILITY, (uptr)&untypedArray[i], i, 0, 0, 0, 0);
		mkmi_log("Got region [0x%x - 0x%x]\r\n", untypedArray[i].Object, untypedArray[i].Object + untypedArray[i].Size);
		if (untypedArray[i].Type != UNTYPED_FRAMES) { usable[i] = false; break; };
		usable[i] = true;
	}
		
	mkmi_log("Total of %d capabilities available\r\n", count);

	InitializeUntypedMemory(untypedArray, usable, count);
	Capability framesUt;
	GetUntypedRegion(PAGE_SIZE * 512, &framesUt);

	Capability framesRetyped[512];
	RetypeCapability(framesUt, framesRetyped, FRAME_MEMORY, 512);
	
	uptr heapAddr = 0x1000;
	heapMapper = MemoryMapper(heapAddr);
	kernelHeap = Heap((uptr)
		heapMapper.MMap(framesRetyped, 512, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE),
		512 * PAGE_SIZE
	);
	

	memoryMapper = MemoryMapper(0x400000000);
	vfs = VFS(&memoryMapper, &kernelHeap);
	RamFS ramfs(&vfs, &memoryMapper, &kernelHeap, 100);
	RamFS devfs(&vfs, &memoryMapper, &kernelHeap, 100);
		

/*
	VFSNodeHandle rootHandle;
	ramfs.Open(0, &rootHandle);
	vfs.Mount(rootHandle, rootHandle);

	VFSNodeHandle rootDevDirHandle;
	ramfs.MkDir(rootHandle, "dev", &rootDevDirHandle);

	VFSNodeHandle devRootHandle;
	devfs.Open(0, &devRootHandle);
	vfs.Mount(rootDevDirHandle, devRootHandle);
*/
	
	InitACPI(&kernelHeap, &memoryMapper, info);

	kernelHeap.DebugDump();

	WriteSerial("Hello, world\r\n");


	while(true) { }
	return 0;
}

