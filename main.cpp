#include <stdint.h>
#include <stddef.h>
#include <mkmi_syscall.h>

extern "C" int Main(void *parent) {
	Syscall(0, (size_t)"Hello, debug syscall.\r\n", 0, 0, 0, 0, 0);

	(void)parent;
	return 0;
}
