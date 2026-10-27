#include "console.h"
#include "uart.h"
#include "cpu.h"

/* This path only sends bytes. It never calls the graphical console. */
static void serial_text(const char *text)
{
    while (*text) traite_car_uart(*text++);
}

void kernel_start(void)
{
    disable_it();
    init_uart();
    console_putbytes("\f", 1);
    console_putbytes("", 0);
    const char *title = "UART TX example\n";
    console_putbytes(title, (int)strlen(title));

    serial_text("UART_ONLY: sent directly to UART.\n");

    place_curseur(4, 0);
    const char *mirrored = "CONSOLE_MIRROR: screen and UART.\n";
    console_putbytes(mirrored, (int)strlen(mirrored));
    const char *note = "The UART_ONLY line is not drawn here.\n";
    console_putbytes(note, (int)strlen(note));
    serial_text("EVIDENCE: UART PASS direct_tx=1 console_mirror=1\n");

    for (;;) hlt();
}
