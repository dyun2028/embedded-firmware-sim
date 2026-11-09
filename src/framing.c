#include "framing.h"
#include "crc.h"

void fp_init(frame_parser_t *p) { p->state = FS_WAIT_SOF; p->len = p->got = 0; p->good = p->bad_crc = p->bad_len = 0; }

frame_result_t fp_feed(frame_parser_t *p, uint8_t b) {
    switch (p->state) {
    case FS_WAIT_SOF:
        if (b == FRAME_SOF) p->state = FS_LEN;
        return FP_NONE;
    case FS_LEN:
        if (b == 0 || b > FRAME_MAX_PAYLOAD) {          /* resync: reject impossible length */
            p->bad_len++; p->state = (b == FRAME_SOF) ? FS_LEN : FS_WAIT_SOF; return FP_BAD_LEN;
        }
        p->len = b; p->got = 0; p->state = FS_PAYLOAD; return FP_NONE;
    case FS_PAYLOAD:
        p->payload[p->got++] = b;
        if (p->got == p->len) p->state = FS_CRC;
        return FP_NONE;
    case FS_CRC: {
        uint8_t tmp[FRAME_MAX_PAYLOAD + 1];
        tmp[0] = p->len;
        for (uint8_t i = 0; i < p->len; i++) tmp[1 + i] = p->payload[i];
        p->state = FS_WAIT_SOF;
        if (crc8(tmp, (size_t)p->len + 1) == b) { p->good++; return FP_FRAME_READY; }
        p->bad_crc++; return FP_BAD_CRC;
    }
    }
    return FP_NONE;
}

size_t frame_encode(const uint8_t *payload, uint8_t len, uint8_t *out) {
    if (len == 0 || len > FRAME_MAX_PAYLOAD) return 0;
    out[0] = FRAME_SOF; out[1] = len;
    for (uint8_t i = 0; i < len; i++) out[2 + i] = payload[i];
    out[2 + len] = crc8(&out[1], (size_t)len + 1);
    return (size_t)len + 3;
}
