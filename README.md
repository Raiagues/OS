# OS — RISC-V

A small operating system written in C and RISC-V assembly for the PCSEF / SEOC course at Ensimag in 2023.

The original **QEMU CEP graphical console** has a 1280 × 720 framebuffer, an 8 × 8 font, a clock and a process-state display. The kernel also implements UART output, timer interrupts, round-robin scheduling, sleep and process termination.

## Run

With the course tools installed:

```sh
./run.sh
```

Three processes sleep for 2, 3 and 4 seconds, twice each, then terminate. `idle` keeps running and the clock continues. Close the QEMU window to exit.

For a fresh setup, see **[Run and verify](docs/08-run-and-verify.md)**. It covers the course tools, a Docker environment with graphical output, and the serial-only alternative using standard QEMU packages.

## Small examples

These examples use the kernel's existing functions, with ordinary text and pixels:

```sh
./run.sh DEMO=pixels    # Alphabet and digits, cycling through colors
./run.sh DEMO=console   # Newline, tab, backspace, carriage return and clear
./run.sh DEMO=scroll    # Write more lines than fit on screen
./run.sh DEMO=uart      # Direct UART output and console output
./run.sh DEMO=clock     # Run the clock with idle only
```

The examples are in [demos.c](src/demos.c). They were added to make the original features easy to run separately. The default process demonstration is in [test_student.c](src/test_student.c).

## How it works

Start with the **[guide](docs/README.md)**: QEMU and RISC-V, boot, pixels, UART, interrupts and processes. Each chapter points to the implementation, gives a small experiment and includes actual output.

The [inventory](docs/01-inventory.md) records the review of five local project copies and 62 commits. UART transmission and the process features were present. No working keyboard input was found; the keyboard-related commit contains an unfinished external-interrupt handler.

## Tests

```sh
sh scripts/course-env.sh make test
sh scripts/course-env.sh python3 scripts/capture_evidence.py
```

The first command tests boot, sleep, termination, preemption and formatting bounds. The second runs the CEP examples and saves real framebuffer captures and UART logs in `build/evidence/`. [Recorded evidence](docs/evidence/) is included in the repository. GitHub Actions runs both toolchain and CEP checks.

## Credits

Student implementation by **Emilly Raiane Rodrigues**, based on the course framework. Original French API names and historical process messages are retained. Documentation, new examples and test diagnostics are in English.

Support code includes Flux/OSKit, CMU/Mach, klibc and bitmap-font credits to Daniel Hepper, Marcel Sondaar and IBM. Original notices remain in the source; [COPYING.GPL-2](COPYING.GPL-2) accompanies the components referring to it.
