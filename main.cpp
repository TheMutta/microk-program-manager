#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include <cdefs.h>
extern "C" void VMMCall(usize syscallNum) {
	asm volatile(
		     "mov %0, %%rax\n\t"
		     "vmmcall\n\t"
		     :
		     : "r"(syscallNum)
		     : "memory", "cc", "rax");
}
extern "C" int Main(int argc, char **argv) {
	(void)argc, (void)argv;


	mkmi_log("Hello from the containerized process.\r\n");
	uptr capability = 0;
	VMMCall(0xDEAD);
	VMMCall(0xB337);
	VMMCall(0x10EF);
	VMMCall(0x7071);
	VMMCall(0x8086);

	uptr  obj= 0;
	__fast_syscall(SYSCALL_VECTOR_SEARCH_CAPABILITY, (uptr)0, UNTYPED_FRAMES, (uptr)&capability, 0, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capability, obj);
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, capability, UNTYPED_FRAMES, (uptr)&obj, 0, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capability, obj);


	return 0;
}

