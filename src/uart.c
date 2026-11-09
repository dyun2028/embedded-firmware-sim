#include "uart.h"

void uart_init(uart_regs_t *u, ring_buffer_t *rx) {
    rb_init(rx);
    u->SR = UART_SR_TXE;
    u->CR = UART_CR_UE | UART_CR_RXNEIE;   /* enable peripheral and rx interrupt */
}

/* Keep the ISR short: drain the hardware register into the ring buffer and clear the overrun flag.
 * No parsing, no printf, no blocking calls here. */
void uart_rx_isr(uart_regs_t *u, ring_buffer_t *rx) {
    if (u->SR & UART_SR_ORE) u->SR &= ~UART_SR_ORE;
    if (u->SR & UART_SR_RXNE) {
        uint8_t b = (uint8_t)(u->DR & 0xFFu);   /* reading DR clears RXNE on real hardware */
        u->SR &= ~UART_SR_RXNE;
        rb_push(rx, b);
    }
}
