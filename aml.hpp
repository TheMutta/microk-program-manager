#pragma once
#include <cdefs.h>

#include "memory.hpp"
#include "acpi.hpp"

void ParseAML(SDTHeader_t *table, Heap *kernelHeap);
