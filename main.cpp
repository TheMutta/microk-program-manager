#include <stdint.h>
#include <stddef.h>
#include <mkmi_syscall.h>

void OutB(uint16_t port, uint8_t val) {
	asm volatile ("out %0, %1" :: "a"(val), "Nd"(port));
}

void OutS(char *string) {
	while(*string) {
		OutB(0xe9, *string++);
	}
}

extern "C" int Main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	
	OutS("Wake up!\r\n");

	Syscall(1, 0, 0, 0, 0, 0, 0);

	OutS("Hello, world!\r\n");

	return 0;
}
