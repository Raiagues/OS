# Kernel regression checks

This directory is a complete kernel test project. It contains its own startup,
drivers, scheduler, runtime, Makefile and launchers. Open `src/main.c` to read
the assertions in execution order.

The checks cover bounded formatting, preemption of a busy loop, sleep deadlines,
explicit exit and termination when a task returns. Successful execution prints
`SELFTEST: ALL PASS` through UART. A failure prints `SELFTEST: FAIL` and stops.

With the RISC-V compiler and standard QEMU installed:

```sh
make PLATFORM=virt run
```

Exit QEMU with Ctrl+A, then X. With local CEP tools, use:

```sh
sh scripts/course-env.sh make PLATFORM=virt run
```

For the simpler implementation examples, use the project's
[step-by-step guide](https://raiagues.github.io/OS/). License and attribution
notices are included in this folder and its source files.
