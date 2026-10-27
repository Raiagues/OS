# 4. Pixels and console

This chapter uses three independent projects: [pixels](../examples/01-pixels/README.md), [console controls](../examples/02-console/README.md) and [scrolling](../examples/03-scroll/README.md). Each folder contains its own entry point, drivers, font, runtime, Makefile and launchers. You can copy a folder elsewhere and build it there.

Start with [the pixel example’s main.c](../examples/01-pixels/src/main.c), then read its local [console.c](../examples/01-pixels/src/console.c), [console.h](../examples/01-pixels/src/console.h) and [font.c](../examples/01-pixels/src/font.c).

## Run the pixel example

From the repository root, with the [local CEP tools configured](08-run-and-verify.md#local-cep-toolchain):

```sh
cd examples/01-pixels
./run.sh
```

For the [Docker setup](08-run-and-verify.md#graphical-run-with-docker), enter the same folder and run `./scripts/run-container.sh`. Both launchers build this folder’s `src/main.c`.

Rows 1–4 each show all 95 printable ASCII characters, from space (32) through `~` (126), including punctuation. The rows are red, green, blue and white, using the normal 8 × 8 font. The screen stays static.

In `kernel_start`, a short loop creates the ASCII string and four calls to `color_text` draw it. A 1,602-character paragraph follows the colored rows. It passes through `console_putbytes` as one string without newline bytes. The console automatically starts a new row after 160 characters; it wraps by character, so a word can cross the right edge.

Compare the [framebuffer capture](evidence/pixels.png) with the [UART log](evidence/pixels.txt). The long paragraph wraps on screen even though the string contains no explicit line breaks.

## From coordinates to an address

CEP exposes its framebuffer at `0x80000000`. It has 1280 columns, 720 rows and four bytes per pixel:

```text
index   = y * 1280 + x
address = 0x80000000 + 4 * index
```

In C, adding an index to a `uint32_t *` already multiplies it by four:

```c
volatile uint32_t *address =
    (volatile uint32_t *)ADRESSE_INITIAL + column + ligne * DISPLAY_WIDTH;
*address = couleur;
```

This is the essential operation in `pixel`. `volatile` keeps the device access observable. The function checks coordinates before writing so an invalid position cannot write beyond video memory.

The framebuffer occupies 3,686,400 bytes. These addresses belong to CEP: on `virt`, `0x80000000` is kernel RAM, so the two platforms need different backends.

## From font bits to a character

`font8x8_basic` contains eight bytes per character. Each byte is one row; each bit is a pixel. For `A`, the table contains:

```text
byte    pixels, bit 0 on the left
0x0c    ..##....
0x1e    .####...
0x33    ##..##..
0x33    ##..##..
0x3f    ######..
0x33    ##..##..
0x33    ##..##..
0x00    ........
```

`ecrit_car(row, column, 'A')` reads those bits and calls `pixel` for each set bit. Text coordinates are converted with `x = column * 8 + bit` and `y = row * 8 + font_row`.

Watch the argument order: `pixel(column, row, color)` uses pixel coordinates; `ecrit_car(row, column, character)` uses text cells. `ecrit_car` draws white. The color example follows the same short bitmap loop with a selected color; it does not change the console API.

There are 160 text columns and 90 physical text rows. Regular output starts at row 1, leaving row 0 reserved. The scrolling example uses it for a header; the complete kernel uses it for the clock and process states. These three static examples have no timer or scheduler.

## From `printf` to the screen

```mermaid
flowchart LR
    A[printf] --> B[console_putbytes]
    B --> C[traite_car]
    C --> D[ecrit_car]
    D --> E[pixel]
    E --> F[Framebuffer]
    B --> G[traite_car_uart]
```

`traite_car` handles both printable characters and control bytes:

| Input | Effect |
| --- | --- |
| Printable ASCII | Draw the character and advance the cursor |
| `\n` | Next line, column 0 |
| `\r` | Column 0 of the current line |
| `\b` | Move back one column when possible |
| `\t` | Next multiple of eight columns, wrapping at the right edge |
| `\f` | Clear the display and move the cursor to row 1, column 0 |

Run the independent console project from the repository root:

```sh
cd examples/02-console
./run.sh
```

Or use `./scripts/run-container.sh` from that folder. Compare the [capture](evidence/console.png) with its [log](evidence/console.txt). Its [main.c](../examples/02-console/src/main.c) contains a linear sequence of short strings such as `AB\bC` and `ABC\rZ`; follow each call into the local [console driver](../examples/02-console/src/console.c).

`place_curseur` moves the underline cursor. `efface_char` clears a cell before overwriting it. Calling `ecrit_car` alone only draws the set font bits; it does not clear the cell first. The font and driver handle ASCII bytes, not Unicode text.

## Scrolling

From the repository root:

```sh
cd examples/03-scroll
./run.sh
```

The Docker command is again `./scripts/run-container.sh` from inside this folder. Open [main.c](../examples/03-scroll/src/main.c): the example writes a header, then prints numbered lines in one loop.

After writing 100 numbered lines, [the capture](evidence/scroll.png) shows lines 12 through 99. [The log](evidence/scroll.txt) contains all 100.

The local [console driver’s `defilement`](../examples/03-scroll/src/console.c) uses `memmove` to shift the text area up by eight pixel rows, leaves row 0 in place and clears the last row. Source and destination overlap, which is why this operation needs `memmove`.

A useful exercise is to change one test string, predict the cursor position, run it, and compare. Each folder includes its recorded output under `evidence/`. The examples and capture script provide observations; the screenshots do not prove every possible coordinate or control-byte sequence.
