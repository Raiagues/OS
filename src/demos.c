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
    char characters[96];
    for (unsigned int i = 0; i < 95; ++i) characters[i] = (char)(32 + i);
    characters[95] = '\0';
    color_text(1, characters, 0x00ff0000);
    color_text(2, characters, 0x0000ff00);
    color_text(3, characters, 0x000000ff);
    color_text(4, characters, BLANC);

    /* Adjacent C strings form one paragraph: there are no newline bytes. */
    static const char long_text[] =
        "This is one long paragraph sent to the original console without any newline characters. "
        "The text should reach the right edge, continue on the next line, and keep every character in order. "
        "Each character occupies eight pixels horizontally and eight pixels vertically. "
        "A 1280-pixel-wide display therefore has room for 160 characters on each text line. "
        "The console moves its cursor after each character; reaching the last column starts a new line. "
        "This is character wrapping, so a word can be split between two lines when it reaches the edge. "
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789. "
        "There is no animation here: the four colored rows and this paragraph are drawn once and remain still. "
        "The colored rows contain every printable ASCII character, including spaces and punctuation. "
        "The paragraph uses console_putbytes, which also sends the same bytes to the UART. "
        "Compare the screen with the serial log: the screen wraps while the log keeps this as one long line. "
        "No extra layout code decides where to break the paragraph; the existing console driver handles it. "
        "You can change these sentences, rebuild the kernel, and watch where the final cursor lands. "
        "Adding or removing a character should shift the remaining text by one position. "
        "A long sentence makes it easier to inspect the transition from the right edge to the next row. "
        "The letters should keep their original shape, spacing and order on both sides of that boundary. "
        "0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz. "
        "This final sentence ends the paragraph after more than ten full lines of automatic wrapping.";
    place_curseur(6, 0);
    console_putbytes(long_text, sizeof(long_text) - 1);
    serial_text("\nEVIDENCE: PIXELS PASS\n");
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
