# 1. Kernel overview

This is a small freestanding RISC-V kernel written in C and assembly. It runs on one RV64 hart in machine mode, draws a graphical text console, sends UART output, and schedules tasks with timer interrupts.

The default **CEP** platform has a 1280 × 720 framebuffer. The **`virt`** backend runs with standard QEMU and uses a serial console. Each platform has its own kernel address and UART driver; [QEMU and RISC-V](02-qemu-riscv.md) explains the difference.

## Architecture

```mermaid
flowchart TD
    E["ELF entry: assembly startup"] --> K["kernel_start: initialize devices"]
    K --> P["Create proc1, proc2 and proc3"]
    P --> I["Enter idle"]
    D["Task functions"] --> C["console_putbytes"]
    C --> F["Font bits → framebuffer pixels"]
    C --> U["UART transmit register"]
    K --> T["Timer interrupt"]
    T --> H["Assembly trap entry → C handler"]
    H --> R["Clock and state display"]
    H --> S["Round-robin scheduler"]
    S --> X["Context switch between task stacks"]
    X --> D
```

Assembly handles startup, trap entry and context switching where explicit register control is necessary. C handles formatting, device access, task state and scheduling policy. Tasks share the kernel's memory and privileges.

## Start with the independent examples

Six numbered projects introduce the mechanisms one at a time. Start with each folder's README, then open its `src/main.c`. Every folder contains all the source, headers, boot code, linker script, Makefile, launchers and recorded output it needs. You can copy any one of these folders and build it independently.

| Project and starting instructions | Main source | What to observe |
| --- | --- | --- |
| [01 — Pixels](../examples/01-pixels/README.md) | [main.c](../examples/01-pixels/src/main.c) | Printable ASCII in four colors and automatic paragraph wrapping: [screen](evidence/pixels.png), [log](evidence/pixels.txt) |
| [02 — Console](../examples/02-console/README.md) | [main.c](../examples/02-console/src/main.c) | Newline, tab, backspace, carriage return and clear: [screen](evidence/console.png), [log](evidence/console.txt) |
| [03 — Scrolling](../examples/03-scroll/README.md) | [main.c](../examples/03-scroll/src/main.c) | Move overlapping framebuffer rows while preserving a header: [screen](evidence/scroll.png), [log](evidence/scroll.txt) |
| [04 — UART](../examples/04-uart/README.md) | [main.c](../examples/04-uart/src/main.c) | Direct serial output and console output mirrored to UART: [screen](evidence/uart.png), [log](evidence/uart.txt) |
| [05 — Clock](../examples/05-clock/README.md) | [main.c](../examples/05-clock/src/main.c) | Timer interrupts and a clock without a scheduler: [screen](evidence/clock.png), [log](evidence/clock.txt) |
| [06 — Processes](../examples/06-processes/README.md) | [main.c](../examples/06-processes/src/main.c) | Sleeping, waking and terminating tasks: [start](evidence/processes-start.png), [end](evidence/processes-end.png), [log](evidence/processes.txt) |

With the [local CEP tools configured](08-run-and-verify.md#local-cep-toolchain), start from the repository root:

```sh
cd examples/01-pixels
./run.sh
```

For Docker, run `./scripts/run-container.sh` inside the selected folder. Close QEMU before starting another project. Each launcher runs the project in its own folder.

The repository root contains the complete kernel. From there, `./run.sh` or `./scripts/run-container.sh` starts the graphical console, clock and three tasks together. The independent process example exposes the same complete task lifecycle in its own project.

## Complete kernel code map

| Component | Source | Responsibility |
| --- | --- | --- |
| Build and memory layout | [Makefile](../Makefile), [kernel.lds](../src/kernel.lds) | Select the platform, compile objects and place ELF sections |
| Startup | [crt0.S](../src/crt0.S), [main.c](../src/main.c) | Set `gp` and `sp`, clear `.bss`, initialize devices and create the tasks |
| Platform constants | [cep_platform.h](../src/cep_platform.h) | Device addresses, timer frequency and register fields |
| Graphical console | [console.c](../src/console.c), [font.c](../src/font.c) | Pixels, 8 × 8 characters, cursor, control bytes and scrolling |
| Serial output | [uart.c](../src/uart.c), [console_serial.c](../src/console_serial.c) | Polling UART transmission and the `virt` console backend |
| Timer and traps | [time.c](../src/time.c), [traitant.S](../src/traitant.S) | Program deadlines, preserve interrupted registers and dispatch the timer interrupt |
| Task management | [processus.c](../src/processus.c), [processus.h](../src/processus.h) | Creation, identity, round-robin selection, sleep and exit |
| Context switching | [ctxt.S](../src/ctxt.S) | Save and restore the task's stack pointer, return address and machine context |
| Task bodies and idle loop | [main.c](../src/main.c) | Define the three task functions, their sleeps and the idle loop |
| Automated test kernel | [tests/kernel](../tests/kernel/README.md), [its main.c](../tests/kernel/src/main.c) | Independent executable for formatting, preemption, sleep and exit checks |
| Host-side verification | [test_qemu.py](../scripts/test_qemu.py), [capture_evidence.py](../scripts/capture_evidence.py) | Run kernels, check output and capture the actual framebuffer |

## Task lifecycle

The fixed table has four slots: `idle` and three other tasks. Each task has a name, PID, state, saved context and stack storage. A normal task reserves a 32 KiB stack; `idle` continues on the 16 KiB boot stack.

| State | Value | Scheduler behavior |
| --- | --- | --- |
| `RUN` | 0 | Task currently executing |
| `RUNNABLE` | 1 | Eligible for the next turn |
| `WAIT` | 2 | Skipped until its sleep deadline is reached |
| `DIE` | 3 | Finished or unused; skipped |

`cree_processus` prepares a task for its first turn. `dors` saves a wakeup deadline and yields. `fin_processus` ends a task; `proc_launcher` also calls it when the task function returns. The timer invokes the scheduler at a nominal 20 Hz, while the visible clock updates once per second. See [Processes](07-processes.md) for the register layout and switching sequence.

## Limits

- One hart, four task slots, no priorities and no reuse of terminated slots.
- Shared memory and M-mode privileges; no user-mode isolation, paging setup or stack-overflow checks.
- Sleep uses whole seconds derived from handled timer interrupts, so it is not a precise wall-clock delay.
- UART transmission is implemented. UART reception, keyboard input and a shell are not implemented.
- No filesystem, networking or system-call interface.

## Verify the behavior

```sh
sh scripts/course-env.sh make test
sh scripts/course-env.sh python3 scripts/capture_evidence.py
```

Run these commands from the repository root. `make test` builds the complete kernel and the independent `tests/kernel` project for `virt`, then checks boot, formatting bounds, sleep, termination and timer preemption. The capture script runs the CEP examples and reads video memory through QMP. Screenshots show output; UART logs record transmitted bytes; the busy-loop test checks preemption. Use each for the behavior it can establish.

Start the implementation walkthrough at [Build the kernel step by step](09-development.md), or use [Run and verify](08-run-and-verify.md) for installation and evidence collection.
