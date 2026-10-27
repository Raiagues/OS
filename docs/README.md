# Project guide

[Read online with screenshots and interactive diagrams](https://raiagues.github.io/OS/).

Follow a character from `printf` to a pixel, or a timer interrupt from the CPU to a process switch. Learn how the kernel fits together, run its examples, and build each mechanism step by step.

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
| [9. Build it step by step](09-development.md) | Implement the boot path, console, timer and scheduler in working stages |

For a first run, start with chapter 8. Use chapter 9 as the implementation walkthrough and chapters 2–7 for the details behind each stage. The diagrams explain control flow; the linked screenshots and logs show actual QEMU executions.

[Back to the repository](../README.md)
