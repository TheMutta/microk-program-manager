#include "initrd.hpp"

int memcmp(const void *ptr1, const void *ptr2, usize count) {
	const unsigned char *s1 = (const unsigned char*)ptr1;
	const unsigned char *s2 = (const unsigned char*)ptr2;

	while (count-- > 0)
	{
		if (*s1++ != *s2++)
			return s1[-1] < s2[-1] ? -1 : 1;
	}
	return 0;
}

usize strlen(const char *str) {
	const char *s;

	for (s = str; *s; ++s);
	return(s - str);
}

unsigned int getsize(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

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

struct tar_header
{
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

int InitrdInstance::IndexInitrd() {
    unsigned char *ptr = Address;

    if(memcmp(ptr + 257, "ustar", 5)) {
	    return -1;
    }

    // Without that 4096 we crash...to be continued
    while((uptr)ptr - (uptr)Address < Size - 4096 && !memcmp(ptr + 257, "ustar", 5)) {
	tar_header *file = (tar_header*)ptr;
        int filesize = getsize(file->size);

	mkmi_log("File: %s of size %d bytes\r\n", file->filename, filesize);
	/*
        if (!memcmp(ptr, filename, strlen(filename) + 1)) {
            *out = ptr + 512;
            return filesize;
        }*/
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
	
    mkmi_log("OK\r\n");

    return 0;
}

int InitrdInstance::UnpackInitrd() {
	return -1;
}

