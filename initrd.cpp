#include "initrd.hpp"

int tar_lookup(u8 *archive, char *filename, u8 **out) {
	u8 *ptr = archive;

	while(!memcmp(ptr + 257, "ustar", 5)) {
		int filesize = oct2bin(ptr + 0x7c, 11);
		if (!memcmp(ptr, filename, strlen(filename))) {
		}
	}

}
InitrdInstance::InitrdInstance(uint8_t *address, size_t size, InitrdFormat_t format)  : Address(address), Size(size), Format(format) {
	if (Format >= INITRD_FORMAT_COUNT) {
		Format = INITRD_TAR_UNCOMPRESSED;
	}


	if (Format == INITRD_TAR_UNCOMPRESSED) {
		// Can use it as is
		IndexInitrd();
	} else {
		// We must unpack it
	}
}

int InitrdInstance::IndexInitrd() {
	return -1;
}

int InitrdInstance::UnpackInitrd() {
	return -1;
}

