#include <stdint.h>
#include <stddef.h>
#include <mkmi_syscall.h>
#include <mkmi_capability.h>

void PutStr(const char *str) {
	Syscall(SYSCALL_VECTOR_DEBUG, (usize)str, 's', 0, 0, 0, 0);
}

void PutHex(usize hex) {
	Syscall(SYSCALL_VECTOR_DEBUG, hex, 'x', 0, 0, 0, 0);
}

void PutDec(long dec) {
	Syscall(SYSCALL_VECTOR_DEBUG, dec, 'd', 0, 0, 0, 0);
}

extern "C" int Main(void *parent) {
	PutStr("Hello, from userland!\r\n");
}
