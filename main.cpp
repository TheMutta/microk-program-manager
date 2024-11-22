#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>
#include <cdefs.h>

extern "C" int Main(int argc, char **argv) {
	(void)argc, (void)argv;

	mkmi_log("Hello from the containerized process.\r\n");
	uptr capability = 0;
	uptr  obj= 0;
	__fast_syscall(SYSCALL_VECTOR_SEARCH_CAPABILITY, (uptr)-1, UNTYPED_FRAMES, (uptr)&capability, 0, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capability, obj);
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, capability, UNTYPED_FRAMES, (uptr)&obj, 0, 0, 0);
	mkmi_log("Capability: 0x%x -> 0x%x\r\n", capability, obj);


	return 0;
}

