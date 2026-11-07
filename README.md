# RISC-V operating system

Operating systems and computer architecture course project, written in C and
RISC-V assembly. The kernel runs on the course's **CEP machine in QEMU** and
implements a graphical console, UART output, timer interrupts and task scheduling.

The activity folders separate the course mechanisms into runnable programs.
They include later build and correctness fixes; they are not unchanged snapshots
of the original submissions.

## Activities

| Activity | Implementation |
| --- | --- |
| [01 Pixels and characters](examples/01-pixels/README.md) | Framebuffer addresses, bitmap fonts and text wrapping |
| [02 Console](examples/02-console/README.md) | Cursor movement and control characters |
| [03 Scrolling](examples/03-scroll/README.md) | Moving overlapping rows in video memory |
| [04 UART](examples/04-uart/README.md) | Serial transmission through device registers |
| [05 Timer interrupts](examples/05-clock/README.md) | Interrupt entry, timer configuration and timekeeping |
| [06 Processes](examples/06-processes/README.md) | Task states, context switching, sleep, wakeup and termination |

Each folder contains its source, Makefile, launch scripts and a README with
code explanations, diagrams, QEMU screenshots and serial output.

## Run an activity

The compiler and custom QEMU from the course are expected under
`/matieres/4MMPCSEA/xinul`. For example, from the repository root:

```sh
cd examples/01-pixels
./run.sh
```

For a different installation:

```sh
COURSE_TOOLS=/path/to/xinul ./run.sh
```

Standard QEMU packages do not provide the CEP machine or its framebuffer.
Alternatively, each activity can use the course tools image through Docker:

```sh
./scripts/run-container.sh
```

This requires Linux x86_64, Docker, Xephyr, xauth and an X11 or XWayland desktop.
On Ubuntu/Debian the packages are `docker.io`, `xserver-xephyr` and `xauth`.
Check `docker info` before launching. The image is downloaded on first use.

## Complete kernel

The root [src](src/) combines the console, timer and processes. Starting from
the repository root:

```sh
sh scripts/course-env.sh make check
sh scripts/course-env.sh make
./run.sh
```

The same Docker launcher is available at the root. `src/main.c` starts three
tasks that print their identities, sleep for 2, 3 and 4 seconds, and finish.
`idle` continues waiting for interrupts. The [process activity](examples/06-processes/README.md)
explains the task table and assembly context switch.

The build produces `build/cep/kernel.elf`. `make -C src` forwards to the root
Makefile for compatibility with the course directory layout.

Tasks share memory and machine-mode privileges on one emulated CPU. The kernel
has no user-mode isolation, filesystem or keyboard input.

## Credits

Emilly Raiane Rodrigues — course project, September–December 2023.

Boot and platform support are based on code by Sébastien Viardot. Runtime code
includes Flux/OSKit, CMU/Mach and klibc. Font credits include Daniel Hepper,
Marcel Sondaar and IBM. Source attribution and license notices are retained;
see [COPYING.GPL-2](COPYING.GPL-2).
