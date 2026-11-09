#include "ring_buffer.h"

void rb_init(ring_buffer_t *rb) { rb->head = rb->tail = rb->dropped = 0; }

size_t rb_count(const ring_buffer_t *rb) { return (uint32_t)(rb->head - rb->tail); }

bool rb_push(ring_buffer_t *rb, uint8_t b) {
    if (rb_count(rb) == RB_CAPACITY) { rb->dropped++; return false; }
    rb->buf[rb->head & (RB_CAPACITY - 1u)] = b;
    rb->head++;
    return true;
}

bool rb_pop(ring_buffer_t *rb, uint8_t *out) {
    if (rb_count(rb) == 0) return false;
    *out = rb->buf[rb->tail & (RB_CAPACITY - 1u)];
    rb->tail++;
    return true;
}
