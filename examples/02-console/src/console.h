#ifndef CONSOLE_H
#define CONSOLE_H
#include "inttypes.h"
#include "string.h"
#include "cpu.h"
#include "uart.h"

#define ADRESSE_INITIAL 0x80000000
#define DISPLAY_WIDTH 1280
#define DISPLAY_HEIGHT 720
#define BITS 8
#define BLANC 0xffffffffU
#define NOIR 0

void pixel(uint32_t column, uint32_t row, uint32_t color);
void ecrit_car(uint32_t row, uint32_t column, char character);
void efface_position(void);
void place_curseur(uint32_t row, uint32_t column);
void efface_char(void);
void efface_ecran(void);
void defilement(void);
void traite_car(char character);
void console_putbytes(const char *text, int length);
#endif
