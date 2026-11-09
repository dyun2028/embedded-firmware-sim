#ifndef CRC_H
#define CRC_H
#include <stdint.h>
#include <stddef.h>
uint8_t crc8(const uint8_t *data, size_t len);    /* poly 0x07, init 0x00 (CRC-8/SMBUS) */
uint16_t crc16_ccitt(const uint8_t *data, size_t len); /* poly 0x1021, init 0xFFFF (CCITT-FALSE) */
#endif
