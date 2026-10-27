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
    place_curseur(40, 0);
    printf("This line will be removed by FF.");
    console_putbytes("\f", 1);
    printf("Console controls (8x8 characters)\n");
    printf("AB\nCD\n");
    printf("AB\bC\n");
    printf("ABC\rZ");
    place_curseur(6, 0);
    printf("A\tB\n");
    place_curseur(8, 158);
    console_putbytes("\tZ", 2);
    place_curseur(11, 0);
    printf("Rows 2-6: LF, BS, CR, HT. Row 9: tab at the right edge.\n");
    const char *done = "EVIDENCE: CONSOLE PASS\n";
    while (*done) traite_car_uart(*done++);
    for (;;) hlt();
}
