#include "crc.h"

uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc = (uint8_t)(crc ^ data[i]);
        for (int b = 0; b < 8; b++) {
            uint8_t shifted = (uint8_t)(crc << 1);
            crc = (crc & 0x80u) ? (uint8_t)(shifted ^ 0x07u) : shifted;
        }
    }
    return crc;
}

uint16_t crc16_ccitt(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc = (uint16_t)(crc ^ (uint16_t)((uint16_t)data[i] << 8));
        for (int b = 0; b < 8; b++) {
            uint16_t shifted = (uint16_t)(crc << 1);
            crc = (crc & 0x8000u) ? (uint16_t)(shifted ^ 0x1021u) : shifted;
        }
    }
    return crc;
}
