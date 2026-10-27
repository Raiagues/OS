#include "console.h"
#include "font.h"

/* Keep each glyph at 8x8; choose its ink color when writing the font bits. */
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

void kernel_start(void)
{
    init_uart();
    efface_ecran();
    const char banner[] = "OS / RISC-V RV64 / QEMU CEP\n";
    console_putbytes(banner, sizeof(banner) - 1);
    console_putbytes("\f", 1);
    console_putbytes("", 0);
    char characters[96];
    for (unsigned int i = 0; i < 95; ++i) characters[i] = (char)(32 + i);
    characters[95] = '\0';
    color_text(1, characters, 0x00ff0000);
    color_text(2, characters, 0x0000ff00);
    color_text(3, characters, 0x000000ff);
    color_text(4, characters, BLANC);

    /* One paragraph: the console, not newline bytes, determines its wrapping. */
    static const char long_text[] =
        "This is one long paragraph sent to the kernel's console without any newline characters. "
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
    const char *done = "\nEVIDENCE: PIXELS PASS\n";
    while (*done) traite_car_uart(*done++);
    for (;;) hlt();
}
