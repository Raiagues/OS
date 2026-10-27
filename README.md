# OS — RISC-V

A small operating system written in C and RISC-V assembly. It runs on one emulated CPU in QEMU, with a graphical console, UART output, timer interrupts and a round-robin scheduler.

**[Tutorials, real screenshots and interactive diagrams](https://raiagues.github.io/OS/)**

## Start with one small project

Each example is a complete project with its own source files, Makefile, launchers, README and captured output. You can copy one folder elsewhere and build it on its own. Open its `src/main.c` first.

| Folder | What to learn |
| --- | --- |
| [01-pixels](examples/01-pixels/README.md) | Pixel addresses, bitmap characters, colors and wrapping |
| [02-console](examples/02-console/README.md) | Cursor movement, newline, tab, backspace and clear |
| [03-scroll](examples/03-scroll/README.md) | Moving text when the screen fills |
| [04-uart](examples/04-uart/README.md) | Sending bytes and comparing serial output with the screen |
| [05-clock](examples/05-clock/README.md) | Timer interrupts and a clock, without a scheduler |
| [06-processes](examples/06-processes/README.md) | Separate stacks, context switching, sleep and exit |

For example, with Docker, Xephyr and xauth installed on Linux/amd64:

```sh
git clone https://github.com/Raiagues/OS.git
cd OS/examples/02-console
./scripts/run-container.sh
```

With CEP tools installed locally, use `./run.sh` in that same folder. Both commands run just the console example. See [installation](docs/08-run-and-verify.md) for dependency setup and [the example index](examples/README.md) for the reading order.

## Run the complete kernel

From the repository root:

```sh
./scripts/run-container.sh
```

Or use `./run.sh` with local CEP tools. Three processes sleep for 2, 3 and 4 seconds, twice each, then terminate. `idle` keeps running and the clock continues. Close the QEMU window to exit.

The complete kernel is in [src](src/), starting at [main.c](src/main.c). Its display uses a 1280 × 720 framebuffer and an 8 × 8 font. Keyboard input and a shell are not implemented; tasks share kernel memory and machine-mode privileges.

## Understand and verify

The [guide](docs/README.md) explains the code, design choices and expected output, from boot to context switching. The [implementation walkthrough](docs/09-development.md) connects the independent examples in order.

With a RISC-V compiler and standard QEMU installed:

```sh
make test
```

This runs the complete kernel and the separate [kernel regression project](tests/kernel/README.md), checking preemption, sleep, termination and formatting bounds. The [capture instructions](docs/08-run-and-verify.md#capture-the-real-framebuffer) regenerate each example's real screenshots and UART logs. GitHub Actions builds the independent projects and runs the checks.

## Credits

Developed by **Emilly Raiane Rodrigues**. Boot and platform support are based on code by Sébastien Viardot.

Support code includes Flux/OSKit, CMU/Mach and klibc; bitmap-font credits include Daniel Hepper, Marcel Sondaar and IBM. Attribution and license notices remain in the source. [COPYING.GPL-2](COPYING.GPL-2) accompanies the components referring to it.
