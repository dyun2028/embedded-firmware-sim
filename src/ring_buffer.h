#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Fixed-size, allocation-free single-producer/single-consumer byte ring buffer.
 * Capacity must be a power of two so wrap-around is a bit mask, not a modulo.
 * head is written only by the producer (ISR), tail only by the consumer (main loop). */
#define RB_CAPACITY 64u
_Static_assert((RB_CAPACITY & (RB_CAPACITY - 1u)) == 0, "RB_CAPACITY must be a power of two");

typedef struct {
    volatile uint8_t buf[RB_CAPACITY];
    volatile uint32_t head;   /* next write index (free-running) */
    volatile uint32_t tail;   /* next read index (free-running)  */
    volatile uint32_t dropped;
} ring_buffer_t;

void rb_init(ring_buffer_t *rb);
bool rb_push(ring_buffer_t *rb, uint8_t b);        /* false (and dropped++) if full */
bool rb_pop(ring_buffer_t *rb, uint8_t *out);      /* false if empty */
size_t rb_count(const ring_buffer_t *rb);
#endif
