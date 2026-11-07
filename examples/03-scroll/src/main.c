#include "console.h"
#include "stdio.h"

void kernel_start(void)
{
    init_uart();
    efface_ecran();
    const char banner[] = "OS / RISC-V RV64 / QEMU CEP\n";
    console_putbytes(banner, sizeof(banner) - 1);
    console_putbytes("\f", 1);
    console_putbytes("", 0);
    const char *header = "Scrolling: 100 numbered lines";
    for (uint32_t col = 0; header[col]; ++col) ecrit_car(0, col, header[col]);
    for (uint32_t i = 0; i < 100; ++i)
        printf("line %02u\n", i);
    for (;;) hlt();
}
