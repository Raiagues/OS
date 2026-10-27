#include "uart.h"

/* CEP's SiFive UART: poll TXFIFO before writing each byte. */
static volatile uint32_t *const uart = (volatile uint32_t *)0x10013000;

void init_uart(void)
{
    uart[6] = 32000000 / 115200 - 1;
    uart[2] = 1;
}

void traite_car_uart(char character)
{
    while (uart[0] & 0x80000000U) { }
    uart[0] = (uint8_t)character;
}
