# OS — RISC-V

A small operating system written in C and RISC-V assembly. It runs on one emulated CPU in QEMU, with a graphical console, UART output, timer interrupts and a round-robin scheduler.

**[Read the tutorials and explore the interactive diagrams](https://raiagues.github.io/OS/)**

The graphical display uses a 1280 × 720 framebuffer and an 8 × 8 bitmap font. Three processes run alongside `idle`, each with its own stack, sleep deadline and saved registers.

## Run

On Linux/amd64 with Docker, Xephyr and xauth installed:

```sh
git clone https://github.com/Raiagues/OS.git
cd OS
./scripts/run-container.sh
```

The launcher opens the QEMU graphical window. Three processes sleep for 2, 3 and 4 seconds, twice each, then terminate. `idle` keeps running and the clock continues. Close the window to exit.

**[Installation and first run](docs/08-run-and-verify.md)** explains the dependencies, Docker setup, local CEP tools and standard-QEMU serial mode. With CEP tools installed locally, use `./run.sh`.

## Try each feature

```sh
./scripts/run-container.sh pixels    # Four static colored ASCII rows and automatic wrapping
./scripts/run-container.sh console   # Newline, tab, backspace, carriage return and clear
./scripts/run-container.sh scroll    # Write more lines than fit on screen
./scripts/run-container.sh uart      # Direct UART output and console output
./scripts/run-container.sh clock     # Run the clock with idle only
```

With local CEP tools, the equivalent is `./run.sh DEMO=pixels`, and so on. The small examples call kernel functions directly in [demos.c](src/demos.c); the [process example](src/test_student.c) exercises scheduling, sleep and termination.

## Understand the implementation

Follow the **[project guide](docs/README.md)** to trace boot, pixel rendering, console output, UART, interrupts and context switching. Each chapter links to the code, explains the design and gives commands with expected results. The [implementation walkthrough](docs/09-development.md) connects these pieces step by step.

The site includes actual QEMU screenshots, UART logs and interactive models of bitmap rendering and task scheduling. Keyboard input and a command shell are not implemented. Tasks share kernel memory and machine-mode privileges.

## Verify

With the RISC-V compiler and standard QEMU installed:

```sh
make test
```

This checks boot, sleep, termination, preemption and formatting bounds. The [capture instructions](docs/08-run-and-verify.md#capture-the-real-framebuffer) show how to save QEMU framebuffer images and UART logs. [Sample output](docs/evidence/) is included, and GitHub Actions runs the kernel tests and CEP captures.

## Credits

Developed by **Emilly Raiane Rodrigues**. Boot and platform support are based on code by Sébastien Viardot.

Support code includes Flux/OSKit, CMU/Mach and klibc; bitmap-font credits include Daniel Hepper, Marcel Sondaar and IBM. Attribution and license notices remain in the source. [COPYING.GPL-2](COPYING.GPL-2) accompanies the components referring to it.
