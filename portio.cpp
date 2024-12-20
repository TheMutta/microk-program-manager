#include "portio.hpp"

void PortUtils::OutPortB(u16 port, u8 data) {
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}

void PortUtils::OutPortW(u16 port, u16 data) {
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (data));
}

void PortUtils::OutPortL(u16 port, u32 data) {
	asm volatile ("outl %1, %0" : : "dN" (port), "a" (data));
}

u8 PortUtils::InPortB(u16 port) {
	u8 ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

u16 PortUtils::InPortW(u16 port) {
	u16 ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

u32 PortUtils::InPortL(u16 port) {
	u32 ret;
	asm volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}
