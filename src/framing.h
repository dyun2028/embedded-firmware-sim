#ifndef FRAMING_H
#define FRAMING_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Frame layout: [0x7E][len][payload ... len bytes][crc8 over len+payload]. len <= FRAME_MAX_PAYLOAD. */
#define FRAME_SOF 0x7Eu
#define FRAME_MAX_PAYLOAD 32u

typedef enum { FS_WAIT_SOF, FS_LEN, FS_PAYLOAD, FS_CRC } frame_state_t;
typedef enum { FP_NONE, FP_FRAME_READY, FP_BAD_LEN, FP_BAD_CRC } frame_result_t;

typedef struct {
    frame_state_t state;
    uint8_t len, got;
    uint8_t payload[FRAME_MAX_PAYLOAD];
    uint32_t good, bad_crc, bad_len;
} frame_parser_t;

void fp_init(frame_parser_t *p);
frame_result_t fp_feed(frame_parser_t *p, uint8_t byte);   /* one byte at a time, as it arrives */
size_t frame_encode(const uint8_t *payload, uint8_t len, uint8_t *out); /* returns bytes written, 0 if too long */
#endif
