# Project guide

[Read online with screenshots and interactive diagrams](https://raiagues.github.io/OS/).

Follow a character from `printf` to a pixel, or a timer interrupt from the CPU to a process switch. The examples call the kernel's existing functions directly.

| Chapter | What to look for |
| --- | --- |
| [1. Inventory](01-inventory.md) | Features recovered from the original copies and history |
| [2. QEMU and RISC-V](02-qemu-riscv.md) | Emulator, instruction set, hart and device addresses |
| [3. Boot](03-boot-kernel.md) | ELF, linker, stack, BSS and the first C function |
| [4. Pixels and console](04-pixels-console.md) | Pixel addresses, font bits, cursor and scrolling |
| [5. UART](05-uart.md) | Memory-mapped registers and serial output |
| [6. Clock and interrupts](06-clock-interrupts.md) | CLINT, CSRs, timer ticks and traps |
| [7. Processes](07-processes.md) | Creation, context, scheduling, sleep and termination |
| [8. Run and verify](08-run-and-verify.md) | Installation, commands, tests and captures |
| [9. Development history](09-development.md) | How the original kernel was built, and what was changed during recovery |

For a first run, start with chapter 8, then read the implementation chapters in order. The diagrams explain control flow; the linked screenshots and logs come from actual QEMU executions.

The kernel's original features are separate from the current recovery work: build fixes, the `virt` backend, launchers, tests and individual examples were added later. Each relevant chapter describes that distinction.

[Back to the repository](../README.md)
