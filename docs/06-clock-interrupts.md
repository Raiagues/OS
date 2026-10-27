# 6. Clock and timer interrupts

Periodic timer interrupts update the clock in the screen's corner. The standalone example focuses on that path. In the complete kernel, the same interrupt also invokes the scheduler and enables task preemption.

## Run the clock on its own

From the repository root:

```bash
cd examples/05-clock
./run.sh
```

With Docker and the display helpers installed, use `./scripts/run-container.sh` from the same folder. The folder contains its own source, boot code, runtime and build files.

Start at [src/main.c](../examples/05-clock/src/main.c). `kernel_start` initializes output, installs the handler and enables the timer. Its loop prints each new second and waits with `wfi`. **There is no process table, `idle` task or scheduler in this example.** Watch the clock advance and compare the [serial output](evidence/clock.txt) with the framebuffer capture:

![Clock running on CEP](evidence/clock.png)

The screenshot shows the timer-only program. GDB inspects its interrupt state; the complete kernel's separate preemption test checks scheduling.

## Three different time scales

| Name | Value | Meaning |
| --- | --- | --- |
| `TIMER_FREQ` | 10,000,000 per second | Assumed frequency of the hardware counter `mtime` |
| `ITFREQ` | 20 per second | Nominal interrupt frequency selected by the kernel |
| `nbr_secondes()` | `ticks / 20` | Whole seconds derived from handled interrupts |

The 10 MHz timer frequency is **not the CPU instruction frequency**. Each interrupt is scheduled 500,000 timer increments ahead: `10,000,000 / 20`, or 50 ms nominally.

[programme_timer](../examples/05-clock/src/time.c) writes the next deadline:

```c
*((volatile uint64_t *)CLINT_TIMER_CMP) =
    *((volatile uint64_t *)CLINT_TIMER) + TIMER_FREQ / ITFREQ;
```

These pointers address device registers. `volatile` requires actual accesses even though ordinary C code does not change the counter.

## Memory-mapped registers and CSRs

`mtime` and `mtimecmp` are accessed through memory. The remaining registers below are CPU **control and status registers**, accessed with instructions such as `csrr` and `csrw`.

| Register | Address or field | Role |
| --- | --- | --- |
| `mtime` | `0x0200bff8` | Current hardware timer count |
| Hart 0 `mtimecmp` | `0x02004000` | Deadline for the next interrupt |
| `mtvec` | CSR | Address of `mon_traitant` |
| `mie.MTIE` | Bit 7 | Enable machine timer interrupts |
| `mstatus.MIE` | Bit 3 | Global interrupt enable in M-mode |
| `mstatus.MPIE` | Bit 7 | Previous interrupt-enable state, used on trap return |
| `mcause` | Bit 63; cause code 7 | Identify an RV64 machine timer interrupt |
| `mepc` | CSR | Address at which interrupted execution will resume |

When `mtime >= mtimecmp`, the timer interrupt becomes pending. The enable bits allow the CPU to take it. Writing a future comparison removes that condition. The top bit of `mcause` distinguishes an interrupt from an exception; cause 7 identifies the machine timer. See the [RISC-V machine-mode specification](https://docs.riscv.org/reference/isa/v20260120/priv/machine.html).

The memory addresses are platform constants in [cep_platform.h](../examples/05-clock/src/cep_platform.h), not variables allocated by the linker.

## Setup and interrupt handling

The standalone program installs `mon_traitant` with `init_traitant_timer`. `enable_timer()` programs the deadline, enables `mie.MTIE`, then enables `mstatus.MIE`. Each handler invocation returns to the same main loop.

```mermaid
sequenceDiagram
    participant T as Main loop
    participant H as CPU and CLINT
    participant A as mon_traitant
    participant C as trap_handler
    H->>H: mtime reaches mtimecmp
    H->>A: Save mepc and mcause, then enter mtvec
    A->>A: Save 16 registers on the stack
    A->>C: Call C handler
    C->>C: Check cause, rearm timer, increment ticks
    C->>C: Every 20 ticks: redraw clock
    C-->>A: Return from C
    A->>T: Restore registers and execute mret
```

[traitant.S](../examples/05-clock/src/traitant.S) saves `ra`, `a0..a7` and `t0..t6`: 16 eight-byte registers, or **128 bytes**. These values must survive an asynchronous interruption even though C functions may overwrite them.

`trap_handler` accepts only the expected timer cause. Other causes print `PANIC` with `mcause`, `mepc` and `mtval`, then halt normal execution.

## From ticks to `hh:mm:ss`

`handle_time_interrupt()` rearms the timer, increments `ticks` and updates the clock every 20 ticks.

On CEP, [ecrit_car_en_haut](../examples/05-clock/src/console.c) draws eight characters at the top right, such as `00:00:03`. Hours wrap modulo 100.

This is a count of **handled interrupts**, rather than a direct conversion of elapsed `mtime`. Each deadline is also relative to the time when the handler rearms it. Delayed handling and periods with interrupts disabled can therefore make the displayed clock fall behind the hardware counter.

## Connect the timer to the complete kernel

The complete kernel's [time.c](../src/time.c) also updates the task-state panel every 20 ticks and calls `ordonnance()` **on every tick**, giving scheduling a nominal 50 ms interval. Its process table must be initialized before interrupts are enabled.

If the scheduler switches tasks, the handler remains suspended on the old task's stack. When that task resumes, it finishes the handler and executes `mret`. [Context switching](07-processes.md) explains the additional saved state. The isolated example does not switch stacks.

`nbr_secondes()` rounds down. In the complete kernel, `dors(2)` at kernel time 0.9 s schedules a wakeup at whole second 2, not necessarily 2.9 s. Those sleep deadlines use the discrete clock; the isolated timer example has no sleep API.

## Inspect the timer in GDB

From the repository root in terminal 1:

```bash
cd examples/05-clock
sh scripts/course-env.sh make debug
```

From the repository root in terminal 2:

```bash
gdb-multiarch examples/05-clock/build/cep/kernel.elf
```

At the GDB prompt:

```gdb
target remote localhost:1234
break trap_handler
continue
print/x $mcause
print/x $mepc
print/x $mie
print/x $mstatus
x/gx 0x0200bff8
x/gx 0x02004000
```

Expect **`mcause = 0x8000000000000007`** and timer enable bit `mie & 0x80`. `mepc` depends on the interrupted instruction. Inside the handler, `mstatus.MIE` is cleared: trap entry saves its previous value in `MPIE` for return. This does not indicate a failed timer setup.

The [recorded timer-register session](evidence/timer-gdb.txt) illustrates these CSR values before rearming the comparison; instruction addresses vary with the executable. GDB pauses execution, so use this inspection to understand register state, not to measure normal timing accuracy.

The standalone clock output demonstrates progression. The complete kernel's [busy-loop preemption test](07-processes.md#prove-preemption) demonstrates that another task can run without the active task yielding.
