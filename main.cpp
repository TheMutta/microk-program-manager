#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>

extern "C" int Main(int argc, char **argv) {
	(void)argc, (void)argv;

	mkmi_log("Hello from the containerized process.\r\n");

	return 0;
}

