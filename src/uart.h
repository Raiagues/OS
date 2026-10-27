#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED
#include <inttypes.h>
#include <cpu.h>
void init_uart(void);
void traite_car_uart(char c);
#endif
