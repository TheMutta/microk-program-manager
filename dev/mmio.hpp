#pragma once
#include <cdefs.h>

class MMIOUtils
{
    public:
        static u8 Read8(u64 address);
        static u16 Read16(u64 address);
        static u32 Read32(u64 address);
        static u64 Read64(u64 address);
        static void Write8(u64 address,u8 value);
        static void Write16(u64 address,u16 value);
        static void Write32(u64 address,u32 value);
        static void Write64(u64 address,u64 value);
};
