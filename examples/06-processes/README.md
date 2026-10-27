# 06 — Processes

This is a complete RV64 kernel project with four tasks: `idle`, `proc1`, `proc2` and `proc3`. A 20 Hz timer drives round-robin scheduling. The CEP display shows the console, clock and task states; UART carries the same console messages.

Every source file, header, boot routine, linker script and launcher used by this project is inside this folder. You can copy the folder elsewhere and build it independently. Start with [src/main.c](src/main.c): it contains the boot sequence and all four task functions directly.

## Run the graphical example

Run these commands from this folder.

If a CEP compiler and QEMU are installed:

```sh
./run.sh
```

The launcher checks `/matieres/4MMPCSEA/xinul`. For a different installation:

```sh
COURSE_TOOLS=/path/to/xinul ./run.sh
```

The selected directory must contain `bin/riscv64-unknown-elf-gcc` and `bin/qemu-system-riscv64`. The QEMU executable must support `-machine cep`; standard distribution QEMU packages do not include that machine. The local wrapper selects `rv64ima` for the bundled GCC 11 compiler.

To build without opening a window:

```sh
./scripts/course-env.sh make check all
```

The executable is `build/cep/kernel.elf`. Temporary files and compatibility libraries stay in this folder's `build/` directory.

### Use the pinned tools image

On Linux/amd64, install Docker using its [installation instructions](https://docs.docker.com/engine/install/), and make sure `docker info` works as your user. On Ubuntu or Debian, install the graphical helpers:

```sh
sudo apt install xserver-xephyr xauth
./scripts/run-container.sh
```

The script builds and runs this folder only. Its pinned image contains the compiler and CEP QEMU; the first download is approximately 700 MB and the unpacked image occupies about 2 GB. Xephyr provides an isolated graphical window with a temporary credential. The container does not receive your main desktop credential.

Container builds use `build/container/cep/`. Close the Xephyr window or press Ctrl+C to stop the temporary container. An X11 or XWayland desktop and free space for Xephyr's temporary keyboard files are required.

## Run with standard QEMU

The same process code also supports QEMU's `virt` machine with serial output. On Ubuntu or Debian:

```sh
sudo apt install make gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf qemu-system-misc
make PLATFORM=virt check all
make PLATFORM=virt run
```

This builds `build/virt/kernel.elf`. Press **Ctrl+A, then X** to exit QEMU's terminal mode. The `virt` backend prints task events but has no graphical clock or state panel.

The Makefile accepts tool and output overrides:

```sh
make PLATFORM=cep CROSS_COMPILE=/path/to/bin/riscv64-unknown-elf- \
  QEMU=/path/to/bin/qemu-system-riscv64 RISCV_ARCH=rv64ima \
  BUILD=build/custom all
```

Use `rv64ima_zicsr`, the default, with current RISC-V compiler packages. Use `rv64ima` with the GCC 11 tools bundled in the pinned image. `make help` lists the available targets.

## Follow the execution

1. `crt0.S` disables interrupts, selects hart 0, sets up `gp` and the boot stack, clears BSS and calls `kernel_start()`.
2. `kernel_start()` initializes UART and the console, prepares the process table and trap vector, creates three tasks, then enables the timer. `idle` occupies PID 0 and starts on the boot stack.
3. Every timer interrupt enters `mon_traitant`, saves registers and calls `trap_handler()`. The handler updates time and calls `ordonnance()`.
4. The scheduler wakes tasks whose deadlines have arrived, chooses the next ready slot and calls `ctx_sw()` when the selected task changes.
5. `dors()` marks the current task `WAIT`. It resumes only after becoming ready and being selected again.
6. A task either calls `fin_processus()` or returns through `proc_launcher()`. Both paths mark its slot `DIE`; that task never resumes.

| Task | Work | Termination |
| --- | --- | --- |
| `idle`, PID 0 | Waits for interrupts and reports completion once | Keeps running |
| `proc1`, PID 1 | Prints its identity, then sleeps for 2 kernel seconds; twice | Returns from its function |
| `proc2`, PID 2 | Prints its identity, then sleeps for 3 kernel seconds; twice | Calls `fin_processus()` |
| `proc3`, PID 3 | Prints its identity, then sleeps for 4 kernel seconds; twice | Returns from its function |

Sleep deadlines use `nbr_secondes()`, which rounds the tick count down to whole seconds. Treat console timestamps as kernel-time observations, not precise wall-clock measurements.

The console and UART retain the function messages printed by the process implementation, such as `Processus`, `Debut du sommeil` and `Fin du processus`. Each task prints its identity twice and terminates once. After all three have finished, expect:

```text
[OS] DEMO COMPLETE: proc1, proc2, proc3 finished; idle running.
```

The CEP clock continues after that message. The task-state digits are `0 = RUN`, `1 = RUNNABLE`, `2 = WAIT`, `3 = DIE`; the final panel is `[0 3 3 3]`. The table has four fixed slots, shared memory and no slot reuse or user-mode isolation.

## Read the implementation

| Files | Responsibility |
| --- | --- |
| [main.c](src/main.c) | Boot orchestration, task bodies and idle loop |
| [processus.c](src/processus.c), [processus.h](src/processus.h) | Task table, creation, scheduling, sleep, termination and state display |
| [ctxt.S](src/ctxt.S) | Save and restore the 18-word task context, including `mstatus` |
| [time.c](src/time.c), [traitant.S](src/traitant.S) | Timer programming, trap dispatch and interrupt register frame |
| [console.c](src/console.c), [font.c](src/font.c) | CEP framebuffer text and scrolling |
| [console_serial.c](src/console_serial.c), [uart.c](src/uart.c) | Serial backend and platform-specific UART access |
| [crt0.S](src/crt0.S), [kernel.lds](src/kernel.lds) | Entry point, memory layout and initial stack |
| Remaining local headers and runtime sources | Freestanding formatting, strings, arithmetic and device definitions |

For a halted boot with a GDB server on `127.0.0.1:1234`, use `make debug` with the same platform and tool overrides as your build. Useful breakpoints are `kernel_start`, `ordonnance`, `ctx_sw` and `fin_processus`.

Third-party notices remain in the runtime and font sources. [COPYING.GPL-2](COPYING.GPL-2) accompanies the files that refer to it.

## Recorded output

![Processes sleeping](evidence/processes-start.png)

![All three processes finished](evidence/processes-end.png)

Compare the [UART log](evidence/processes.txt) with the state display. The
[capture report](evidence/report.json) identifies the source files and tools.
