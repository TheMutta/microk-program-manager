#include "netutils.hpp"

u16 flip_short(u16 short_int) {
    u32 first_byte = *((u8*)(&short_int));
    u32 second_byte = *((u8*)(&short_int) + 1);
    return (first_byte << 8) | (second_byte);
}

u32 flip_long(u32 long_int) {
    u32 first_byte = *((u8*)(&long_int));
    u32 second_byte = *((u8*)(&long_int) + 1);
    u32 third_byte = *((u8*)(&long_int)  + 2);
    u32 fourth_byte = *((u8*)(&long_int) + 3);
    return (first_byte << 24) | (second_byte << 16) | (third_byte << 8) | (fourth_byte);
}

/*
 * Flip two parts within a byte
 * For example, 0b11110000 will be 0b00001111 instead
 * This is necessary because endiness is also relevant to byte, where there are two fields in one byte.
 * number_bits: number of bits of the less significant field
 * */
u8 flip_byte(u8 byte, int num_bits) {
    u8 t = byte << (8 - num_bits);
    return t | (byte >> num_bits);
}

u8 htonb(u8 byte, int num_bits) {
    return flip_byte(byte, num_bits);
}

u8 ntohb(u8 byte, int num_bits) {
    return flip_byte(byte, 8 - num_bits);
}


u16 htons(u16 hostshort) {
    return flip_short(hostshort);
}

u32 htonl(u32 hostlong) {
    return flip_long(hostlong);
}

u16 ntohs(u16 netshort) {
    return flip_short(netshort);
}

u32 ntohl(u32 netlong) {
    return flip_long(netlong);
}

