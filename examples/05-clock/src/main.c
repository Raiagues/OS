#include "console.h"
#include "uart.h"
#include "time.h"

void kernel_start(void)
{
    disable_it();
    init_uart();
    console_putbytes("\f", 1);
    console_putbytes("", 0);
    printf("Clock and timer: 20 interrupts per second.\n");
    init_traitant_timer(mon_traitant);
    enable_timer();

    uint32_t previous = 0xffffffffU;
    for (;;) {
        uint32_t seconds = nbr_secondes();
        if (seconds != previous) {
            printf("clock = %u s\n", seconds);
            previous = seconds;
        }
        hlt();
    }
}
