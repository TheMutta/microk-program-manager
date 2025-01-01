#include "portio.hpp"

void PortUtils::OutPortB(u16 port, u8 data) {
	asm volatile("outb %b0, %w1" : : "a"(data), "Nd"(port) : "memory");
}

void PortUtils::OutPortW(u16 port, u16 data) {
	asm volatile("outw %w0, %w1" : : "a"(data), "Nd"(port) : "memory");
}

void PortUtils::OutPortL(u16 port, u32 data) {
	asm volatile("outl %0, %w1" : : "a"(data), "Nd"(port) : "memory");
}

u8 PortUtils::InPortB(u16 port) {
	u8 ret;
	asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

u16 PortUtils::InPortW(u16 port) {
	u16 ret;
	asm volatile("inw %w1, %w0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

u32 PortUtils::InPortL(u16 port) {
	u32 ret;
	asm volatile("inl %w1, %0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}
