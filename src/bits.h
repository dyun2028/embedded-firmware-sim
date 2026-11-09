#ifndef BITS_H
#define BITS_H
#include <stdint.h>
#define BIT(n)            (1u << (n))
#define SET_BITS(r, m)    ((r) |= (m))
#define CLEAR_BITS(r, m)  ((r) &= ~(m))
#define READ_FIELD(r, shift, width) (((r) >> (shift)) & ((1u << (width)) - 1u))
#define WRITE_FIELD(r, shift, width, v) \
    ((r) = ((r) & ~(((1u << (width)) - 1u) << (shift))) | (((uint32_t)(v) & ((1u << (width)) - 1u)) << (shift)))
static inline uint32_t bswap32(uint32_t x) { return (x >> 24) | ((x >> 8) & 0xFF00u) | ((x << 8) & 0xFF0000u) | (x << 24); }
#endif
