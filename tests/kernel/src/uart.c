#include "uart.h"

#ifdef PLATFORM_CEP
/* SiFive UART used by the course's custom machine. */
static volatile uint32_t *const uart = (volatile uint32_t *)0x10013000;
void init_uart(void)
{
    uart[6] = UART_CLOCK_FREQ / UART_BAUD_RATE - 1;
    uart[2] = 1;
}
void traite_car_uart(char c)
{
    while (uart[0] & 0x80000000U) { }
    uart[0] = (uint8_t)c;
}
#else
/* QEMU virt exposes an NS16550A UART with byte-wide registers. */
static volatile uint8_t *const uart = (volatile uint8_t *)0x10000000;
void init_uart(void)
{
    uart[1] = 0;     /* polling, no UART interrupts */
    uart[3] = 0x80;  /* divisor latch */
    uart[0] = 1;
    uart[1] = 0;
    uart[3] = 3;     /* 8 data bits, no parity, one stop bit */
    uart[2] = 7;     /* enable and clear FIFOs */
}
void traite_car_uart(char c)
{
    while (!(uart[5] & 0x20)) { }
    uart[0] = (uint8_t)c;
}
#endif
