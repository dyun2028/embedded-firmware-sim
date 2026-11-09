# Embedded Firmware Sim (C)

Firmware-style C modules written the way MCU code is written (fixed-size buffers, no heap, short ISRs,
memory-mapped register structs, fixed-point math) and unit-tested on a desktop machine.
This is a host-side simulation and study project. It has **not** been run on real microcontroller hardware.

| Module | What it shows |
|---|---|
| `ring_buffer` | Lock-free SPSC byte queue, power-of-two mask indexing, free-running counters (tested across 2^32 wrap), overflow counter |
| `uart` | Memory-mapped register block (SR/DR/CR), receive-interrupt handler that only drains the register into the buffer, overrun flag handling |
| `framing` | Byte-at-a-time state machine for `[SOF][len][payload][CRC-8]` frames with resynchronisation after garbage or bad length |
| `crc` | CRC-8 and CRC-16/CCITT, checked against the published check values for "123456789" |
| `pid` | Q16.16 fixed-point PID with output clamping and anti-windup, tested against a first-order plant |
| `scheduler` | Tick-driven cooperative scheduler: ISR sets flags, main loop runs tasks, overrun detection |
| `bits` | Register bit/field macros and byte swap |

`tests/test_all.c` includes an end-to-end test: bytes go through the simulated UART ISR into the ring buffer, then the main loop feeds the frame parser.

## Build and test
```
make test      # gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror + AddressSanitizer + UBSan
```
Currently 2,234 checks, 0 failures.
