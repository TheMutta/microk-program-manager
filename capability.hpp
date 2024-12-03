#pragma once
#include <object.hpp>

void AddressCapability(uptr object, Capability *cap);
void SplitCapability(Capability capability, Capability *capabilities, usize splitCount, usize splitSize);
void RetypeCapability(Capability capability, Capability *result, OBJECT_TYPE kind);

