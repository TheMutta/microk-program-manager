#include "capability.hpp"
#include <mkmi.h>

void AddressCapability(uptr object, Capability *cap) {
	__fast_syscall(SYSCALL_VECTOR_ADDRESS_CAPABILITY, object, (uptr)cap, 0, 0, 0, 0);
}

void SplitCapability(Capability capability, Capability *capabilities, usize splitCount, usize splitSize) {
	__fast_syscall(SYSCALL_VECTOR_SPLIT_CAPABILITY, capability.Object, (uptr)capabilities, splitSize, 0, splitCount, 0);
}

void RetypeCapability(Capability capability, Capability *result, OBJECT_TYPE kind) {
	__fast_syscall(SYSCALL_VECTOR_RETYPE_CAPABILITY, capability.Object, kind, (uptr)result, 1,  0, 0);
}
