#pragma once
#include <cdefs.h>

class PortUtils {
	public:
		static void OutPortB(u16 port,u8 data);
		static void OutPortW(u16 port,u16 data);
		static void OutPortL(u16 port,u32 data);
		static u8 InPortB(u16 port);
		static u16 InPortW(u16 port);
		static u32 InPortL(u16 port);
};


