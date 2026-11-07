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

    /* Write beyond the last column to observe automatic wrapping. */
    const char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789 ";
    place_curseur(6, 0);
    for (int i = 0; i < 4; ++i) {
        console_putbytes(text, sizeof(text) - 1);
    }
    for (;;) hlt();
}
