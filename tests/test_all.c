#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/ring_buffer.h"
#include "../src/crc.h"
#include "../src/uart.h"
#include "../src/framing.h"
#include "../src/pid.h"
#include "../src/scheduler.h"
#include "../src/bits.h"

static int fails = 0, checks = 0;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)

static void t_crc(void) {
    const uint8_t s[] = "123456789";
    CHECK(crc8(s, 9) == 0xF4);            /* published CRC-8/SMBUS check value */
    CHECK(crc16_ccitt(s, 9) == 0x29B1);   /* published CRC-16/CCITT-FALSE check value */
}

static void t_ring(void) {
    ring_buffer_t rb; rb_init(&rb); uint8_t v;
    CHECK(!rb_pop(&rb, &v));
    for (unsigned i = 0; i < RB_CAPACITY; i++) CHECK(rb_push(&rb, (uint8_t)i));
    CHECK(!rb_push(&rb, 0xAA) && rb.dropped == 1);      /* full: drop and count */
    for (unsigned i = 0; i < RB_CAPACITY; i++) { CHECK(rb_pop(&rb, &v) && v == (uint8_t)i); }
    /* wrap-around: push/pop far past the capacity so the free-running indices overflow the mask many times */
    for (unsigned i = 0; i < 1000; i++) { CHECK(rb_push(&rb, (uint8_t)(i * 7))); CHECK(rb_pop(&rb, &v) && v == (uint8_t)(i * 7)); }
    CHECK(rb_count(&rb) == 0);
    rb.head = rb.tail = 0xFFFFFFF0u;                    /* index counter wrap at 2^32 */
    for (int i = 0; i < 40; i++) CHECK(rb_push(&rb, (uint8_t)i));
    CHECK(rb_count(&rb) == 40);
    for (int i = 0; i < 40; i++) { CHECK(rb_pop(&rb, &v) && v == (uint8_t)i); }
}

static void t_uart_isr(void) {
    uart_regs_t u; ring_buffer_t rx; uart_init(&u, &rx);
    CHECK((u.CR & UART_CR_UE) && (u.CR & UART_CR_RXNEIE));
    for (int i = 0; i < 5; i++) { u.DR = (uint32_t)(0x40 + i); u.SR |= UART_SR_RXNE; uart_rx_isr(&u, &rx); }
    CHECK(rb_count(&rx) == 5 && !(u.SR & UART_SR_RXNE));
    u.SR |= UART_SR_ORE; uart_rx_isr(&u, &rx);
    CHECK(!(u.SR & UART_SR_ORE));                       /* overrun flag cleared */
}

static size_t feed_all(frame_parser_t *p, const uint8_t *b, size_t n, size_t *ready) {
    size_t r = 0; for (size_t i = 0; i < n; i++) if (fp_feed(p, b[i]) == FP_FRAME_READY) r++; if (ready) *ready = r; return r;
}

static void t_framing(void) {
    frame_parser_t p; fp_init(&p);
    uint8_t pay[] = {1, 2, 3, 0x7E, 5}, f[64];
    size_t n = frame_encode(pay, 5, f); CHECK(n == 8);
    size_t ready; feed_all(&p, f, n, &ready);
    CHECK(ready == 1 && p.len == 5 && memcmp(p.payload, pay, 5) == 0);   /* 0x7E inside payload is fine: length-delimited */
    /* corrupt one payload byte -> bad CRC */
    f[3] ^= 0x10; feed_all(&p, f, n, &ready); CHECK(ready == 0 && p.bad_crc == 1);
    /* garbage, then a good frame: the parser must resynchronise */
    uint8_t noise[] = {0x00, 0xFF, 0x13, 0x7E, 0x99 /* impossible len */}, g[64];
    size_t gn = frame_encode(pay, 3, g);
    feed_all(&p, noise, sizeof noise, NULL); feed_all(&p, g, gn, &ready);
    CHECK(ready == 1 && p.good == 2 && p.bad_len == 1);
    CHECK(frame_encode(pay, 0, g) == 0 && frame_encode(pay, FRAME_MAX_PAYLOAD + 1, g) == 0);
}

/* End-to-end: bytes arrive one at a time through the UART ISR into the ring buffer; the main loop drains it into the parser. */
static void t_end_to_end(void) {
    uart_regs_t u; ring_buffer_t rx; frame_parser_t p; uart_init(&u, &rx); fp_init(&p);
    uint8_t pay[] = {'h', 'e', 'l', 'l', 'o'}, f[64]; size_t n = frame_encode(pay, 5, f), got = 0; uint8_t b;
    for (size_t i = 0; i < n; i++) {
        u.DR = f[i]; u.SR |= UART_SR_RXNE; uart_rx_isr(&u, &rx);          /* "interrupt" */
        while (rb_pop(&rx, &b)) if (fp_feed(&p, b) == FP_FRAME_READY) got++; /* main loop */
    }
    CHECK(got == 1 && memcmp(p.payload, "hello", 5) == 0 && rx.dropped == 0);
}

static void t_pid(void) {
    pid_t_ c; pid_init(&c, Q16(0.6), Q16(0.05), Q16(0.1), Q16(-100), Q16(100));
    /* first-order plant: x += (u - x) * 0.2 per tick; drive it to a setpoint of 50 */
    q16_t x = 0; for (int i = 0; i < 400; i++) { q16_t u = pid_step(&c, Q16(50), x); x += q16_mul(u - x, Q16(0.2)); }
    CHECK(abs(x - Q16(50)) < Q16(0.5));                                    /* settles within 0.5 */
    /* saturation + anti-windup: with an unreachable setpoint the output pins at the limit and the integrator stops growing */
    pid_t_ s; pid_init(&s, Q16(1), Q16(1), 0, Q16(-10), Q16(10)); q16_t o = 0;
    for (int i = 0; i < 200; i++) o = pid_step(&s, Q16(1000), 0);
    CHECK(o == Q16(10) && s.integ < Q16(2000));
    CHECK(q16_mul(Q16(2.5), Q16(-4)) == Q16(-10));                         /* signed fixed-point multiply */
}

static int a_runs, b_runs;
static void task_a(void *c) { (void)c; a_runs++; }
static void task_b(void *c) { (void)c; b_runs++; }

static void t_sched(void) {
    scheduler_t s; sched_init(&s); a_runs = b_runs = 0;
    CHECK(sched_add(&s, task_a, NULL, 1) == 0 && sched_add(&s, task_b, NULL, 10) == 1 && sched_add(&s, task_a, NULL, 0) == -1);
    for (int i = 0; i < 100; i++) { sched_tick(&s); sched_run(&s); }
    CHECK(a_runs == 100 && b_runs == 10 && s.tasks[0].overruns == 0);
    /* tick twice without running the main loop -> overrun is detected */
    sched_tick(&s); sched_tick(&s); CHECK(s.tasks[0].overruns == 1);
}

static void t_bits(void) {
    uint32_t r = 0; SET_BITS(r, BIT(3) | BIT(7)); CHECK(r == 0x88);
    CLEAR_BITS(r, BIT(3)); CHECK(r == 0x80);
    WRITE_FIELD(r, 8, 4, 0xA); CHECK(READ_FIELD(r, 8, 4) == 0xA && r == 0xA80);
    WRITE_FIELD(r, 8, 4, 0x1F); CHECK(READ_FIELD(r, 8, 4) == 0xF && (r & 0xF) == 0);  /* value masked to field width, neighbours untouched */
    CHECK(bswap32(0x11223344u) == 0x44332211u);
}

int main(void) {
    t_crc(); t_ring(); t_uart_isr(); t_framing(); t_end_to_end(); t_pid(); t_sched(); t_bits();
    printf("%d checks, %d failures\n", checks, fails);
    return fails != 0;
}
