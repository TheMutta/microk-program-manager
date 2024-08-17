#pragma once
#include <cdefs.h>
#include <stdint.h>
#include <stddef.h>
#include <mkmi.h>

enum InitrdFormat_t {
	INITRD_TAR_UNCOMPRESSED,
	INITRD_TAR_GZ,
	INITRD_TAR_XZ,
	INITRD_FORMAT_COUNT,
};

class InitrdInstance {
public:
	InitrdInstance(uint8_t *address, size_t size, InitrdFormat_t format);

	int SearchForPath(const char *path, uint8_t **returnAddress, size_t *returnSize);

private:
	int IndexInitrd();
	int UnpackInitrd();

	uint8_t *Address;
	size_t Size;
	InitrdFormat_t Format;
};
