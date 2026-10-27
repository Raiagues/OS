#include "uart.h"

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
