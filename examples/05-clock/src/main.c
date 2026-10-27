#include "console.h"
#include "uart.h"
#include "time.h"

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
    printf("Clock and timer: 20 interrupts per second.\n");
    init_traitant_timer(mon_traitant);
    enable_timer();

    uint32_t previous = 0xffffffffU;
    for (;;) {
        uint32_t seconds = nbr_secondes();
        if (seconds != previous) {
            char message[64];
            printf("clock=%us | timer interrupts=20 Hz\n", seconds);
            snprintf(message, sizeof(message), "EVIDENCE: CLOCK SECOND=%u\n", seconds);
            serial_text(message);
            if (seconds == 3) serial_text("EVIDENCE: CLOCK PASS\n");
            previous = seconds;
        }
        hlt();
    }
}
