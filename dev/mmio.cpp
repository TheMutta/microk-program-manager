#include "mmio.hpp"

u8 MMIOUtils::Read8(u64 address) {
    return *((volatile u8*)(address));
}

u16 MMIOUtils::Read16(u64 address) {
    return *((volatile u16*)(address));
    
}

u32 MMIOUtils::Read32(u64 address) {
    return *((volatile u32*)(address));
    
}

u64 MMIOUtils::Read64(u64 address) {
    return *((volatile u64*)(address));    
}

void MMIOUtils::Write8(u64 address,u8 value) {
    (*((volatile u8*)(address)))=(value);
}

void MMIOUtils::Write16(u64 address,u16 value) {
    (*((volatile u16*)(address)))=(value);    
}

void MMIOUtils::Write32(u64 address,u32 value) {
    (*((volatile u32*)(address)))=(value);
    
}

void MMIOUtils::Write64(u64 address,u64 value) {
    (*((volatile u64*)(address)))=(value);    
}
