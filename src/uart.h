#ifndef UART_H
#define UART_H
#include <stdint.h>
#include "ring_buffer.h"

/* A memory-mapped UART register block, modelled the way a real MCU datasheet describes it.
 * On hardware this struct would sit at a fixed peripheral address; here the tests point at a RAM instance. */
typedef struct {
    volatile uint32_t SR;    /* status:  bit0 RXNE (rx data ready), bit1 TXE (tx empty), bit2 ORE (overrun) */
    volatile uint32_t DR;    /* data register: read = rx byte (clears RXNE), write = tx byte */
    volatile uint32_t CR;    /* control: bit0 UE (enable), bit1 RXNEIE (rx interrupt enable) */
} uart_regs_t;

#define UART_SR_RXNE (1u << 0)
#define UART_SR_TXE  (1u << 1)
#define UART_SR_ORE  (1u << 2)
#define UART_CR_UE     (1u << 0)
#define UART_CR_RXNEIE (1u << 1)

void uart_init(uart_regs_t *u, ring_buffer_t *rx);
void uart_rx_isr(uart_regs_t *u, ring_buffer_t *rx);   /* body of the receive interrupt handler */
#endif
