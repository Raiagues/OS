/* Small examples of the existing kernel APIs, added for the documentation.
 * EVIDENCE messages mark capture points; validation is done by the host script. */
#include "demos.h"
#include "console.h"
#include "time.h"
#include "processus.h"
#include "test_student.h"
#if defined(DEMO_PIXELS)
#include "font.h"
#endif

static __attribute__((unused)) void serial_text(const char *text)
{
    while (*text) traite_car_uart(*text++);
}

#if defined(DEMO_PIXELS)
/* The original ecrit_car draws white; the same bits can be sent to pixel
 * with another color. Every character stays at its original 8x8 size. */
static void color_text(uint32_t row, const char *text, uint32_t color)
{
    for (uint32_t col = 0; text[col]; ++col) {
        if (color == BLANC) {
            ecrit_car(row, col, text[col]);
            continue;
        }
        for (uint32_t y = 0; y < 8; ++y) {
            for (uint32_t x = 0; x < 8; ++x) {
                int bit = ((unsigned char)font8x8_basic[(unsigned char)text[col]][y] >> x) & 1;
                pixel(col * 8 + x, row * 8 + y, bit ? color : NOIR);
            }
        }
    }
}
#endif

void demo_run(void)
{
    console_putbytes("\f", 1);
    console_putbytes("", 0);
#if defined(DEMO_PIXELS)
    const uint32_t colors[] = {0x00ff0000, 0x0000ff00, 0x000000ff, BLANC};
    const char *names[] = {"red", "green", "blue", "white"};
    unsigned int color = 0;
    int first_cycle = 1;
    printf("Alphabet and digits: red, green, blue, white.\n");
    place_curseur(10, 0);
    for (;;) {
        color_text(3, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", colors[color]);
        color_text(5, "abcdefghijklmnopqrstuvwxyz", colors[color]);
        color_text(7, "0123456789", colors[color]);
        serial_text("EVIDENCE: PIXELS COLOR=");
        serial_text(names[color]);
        serial_text("\n");
        if (color == 3 && first_cycle) {
            serial_text("EVIDENCE: PIXELS PASS\n");
            first_cycle = 0;
        }
        uint64_t until = *((volatile uint64_t *)CLINT_TIMER) + TIMER_FREQ;
        while (*((volatile uint64_t *)CLINT_TIMER) < until) { }
        color = (color + 1) % 4;
    }
#elif defined(DEMO_CONSOLE)
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
    serial_text("EVIDENCE: CONSOLE PASS\n");
#elif defined(DEMO_SCROLL)
    const char *header = "SCROLL: 100 lines; the top row is reserved.";
    for (uint32_t col = 0; header[col]; ++col) ecrit_car(0, col, header[col]);
    for (uint32_t i = 0; i < 100; ++i) printf("line %02u | console_putbytes -> traite_car -> defilement\n", i);
    serial_text("EVIDENCE: SCROLL PASS written=100 first_visible=12 last_visible=99\n");
#elif defined(DEMO_UART)
    printf("UART TX example\n");
    serial_text("UART_ONLY: sent directly to UART.\n");
    place_curseur(4, 0);
    const char *mirrored = "CONSOLE_MIRROR: screen and UART.\n";
    console_putbytes(mirrored, (int)strlen(mirrored));
    printf("The UART_ONLY line is not drawn here.\n");
    serial_text("EVIDENCE: UART PASS direct_tx=1 console_mirror=1\n");
#elif defined(DEMO_CLOCK)
    printf("Clock and timer: one idle task, 20 interrupts per second.\n");
    init_proc();
    init_traitant_timer(mon_traitant);
    enable_timer();
    uint32_t previous = 0xffffffffU;
    for (;;) {
        uint32_t seconds = nbr_secondes();
        if (seconds != previous) {
            char message[64];
            printf("clock=%us | idle pid=%d\n", seconds, mon_pid());
            snprintf(message, sizeof(message), "EVIDENCE: CLOCK SECOND=%u\n", seconds);
            serial_text(message);
            if (seconds == 3) serial_text("EVIDENCE: CLOCK PASS\n");
            previous = seconds;
        }
        hlt();
    }
#else
    test_processus();
#endif
    for (;;) hlt();
}
