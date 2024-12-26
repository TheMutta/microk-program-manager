#pragma once
#include <cdefs.h>

struct NetworkCard {
	u8 MAC[6];

	void (*SendPacket)(NetworkCard *device, u8 *packet, usize len);
};

u16 flip_short(u16 short_int);

u32 flip_long(u32 long_int);

u8 flip_byte(u8 byte, int num_bits);

u8 htonb(u8 byte, int num_bits);

u8 ntohb(u8 byte, int num_bits);

u16 htons(u16 hostshort);

u32 htonl(u32 hostlong);

u16 ntohs(u16 netshort);

u32 ntohl(u32 netlong);
