# Project guide

[Read online with screenshots and interactive diagrams](https://raiagues.github.io/OS/).

Learn by running six small projects in order. Each folder is self-contained: it has its own `src/main.c`, sources, Makefile, launchers and captured output. Begin with its README, then follow the code from `kernel_start`.

| Start here | Mechanism |
| --- | --- |
| [01 — Pixels](../examples/01-pixels/README.md) | Font bits, colors and paragraph wrapping |
| [02 — Console](../examples/02-console/README.md) | Cursor movement and control characters |
| [03 — Scrolling](../examples/03-scroll/README.md) | Moving and clearing framebuffer rows |
| [04 — UART](../examples/04-uart/README.md) | Sending bytes directly and through the console |
| [05 — Clock](../examples/05-clock/README.md) | Timer interrupts without a scheduler |
| [06 — Processes](../examples/06-processes/README.md) | Task creation, context switches, sleep and exit |

For example, from the repository root with local CEP tools configured:

```sh
cd examples/01-pixels
./run.sh
```

With Docker, use `./scripts/run-container.sh` inside the same folder. [Run and verify](08-run-and-verify.md) covers installation. [Build it step by step](09-development.md) connects the six projects; the chapters below explain each mechanism in detail.

| Chapter | What to look for |
| --- | --- |
| [1. Kernel overview](01-inventory.md) | Components, supported platforms, capabilities and limits |
| [2. QEMU and RISC-V](02-qemu-riscv.md) | Emulator, instruction set, hart and device addresses |
| [3. Boot](03-boot-kernel.md) | ELF, linker, stack, BSS and the first C function |
| [4. Pixels and console](04-pixels-console.md) | Pixel addresses, font bits, cursor and scrolling |
| [5. UART](05-uart.md) | Memory-mapped registers and serial output |
| [6. Clock and interrupts](06-clock-interrupts.md) | CLINT, CSRs, timer ticks and traps |
| [7. Processes](07-processes.md) | Creation, context, scheduling, sleep and termination |
| [8. Run and verify](08-run-and-verify.md) | Installation, commands, tests and captures |
| [9. Build it step by step](09-development.md) | Work through the six independent projects, from pixels to processes |

The repository root contains the complete kernel. Its [main.c](../src/main.c) defines the entry point and task functions. The separate [kernel test project](../tests/kernel/README.md) checks formatting, preemption, sleep and exit; `make test` at the repository root runs it alongside the normal kernel. Diagrams explain control flow, while screenshots and logs show actual QEMU executions.

[Back to the repository](../README.md)
