#include <cdefs.h>

#include <mkmi.h>

#include "memory.hpp"

extern Heap kernelHeap;

extern "C" {

void laihost_log(int level, const char *msg) {
	mkmi_log(msg);
}

__attribute__((noreturn)) void laihost_panic(const char *msg) {
	mkmi_log(msg);
	while(true) { }
}

void *laihost_malloc(usize size) {
	return kernelHeap.Malloc(size);
}

void *laihost_realloc(void *ptr, usize size) {
	return kernelHeap.Realloc(ptr, size);
}

void *laihost_free(void *ptr) {
	return kernelHeap.Free(ptr);
	
}
}
