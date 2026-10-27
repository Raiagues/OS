/* Serial backend for upstream QEMU virt. The CEP framebuffer backend
 * remains in console.c and is selected only by PLATFORM=cep. */
#include "console.h"

void console_putbytes(const char *s, int len)
{
    unsigned long previous;
    __asm__ volatile("csrrc %0, mstatus, %1" : "=r"(previous)
                     : "r"((unsigned long)MSTATUS_MIE) : "memory");
    for (int i = 0; i < len; ++i) {
        if (s[i] == '\n') traite_car_uart('\r');
        if (s[i] != '\0') traite_car_uart(s[i]);
    }
    if (previous & MSTATUS_MIE) enable_it();
}

/* Graphical overlays have no framebuffer on virt. They are intentionally
 * disabled; process/timer events are printed by the demonstration. */
void pixel(uint32_t column, uint32_t row, uint32_t color)
{ (void)column; (void)row; (void)color; }
void ecrit_car(uint32_t row, uint32_t column, char c)
{ (void)row; (void)column; (void)c; }
void efface_position(void) { }
void place_curseur(uint32_t row, uint32_t column)
{ (void)row; (void)column; }
void efface_char(void) { }
void efface_ecran(void) { }
void defilement(void) { }
void traite_car(char c) { console_putbytes(&c, 1); }
void ecrit_car_en_haut(char *timer, int len)
{ (void)timer; (void)len; }
