# 3. From ELF to the first C function

A normal application inherits a stack and initialized memory from its operating system. This kernel must prepare that environment itself before calling C.

## 1. Build and inspect the executable

From the repository root:

```bash
sh scripts/course-env.sh make
sh scripts/course-env.sh sh -c '"${CROSS_COMPILE}readelf" -h -l build/cep/kernel.elf'
```

```mermaid
flowchart LR
    S["C and assembly"] --> O["Object files"]
    O --> L["Linker + kernel.lds"]
    L --> E["kernel.elf"]
    E --> Q["QEMU loads segments"]
    Q --> A["entry: assembly"]
    A --> C["kernel_start: C"]
```

**ELF** stores code, data, load addresses and symbols. Symbols let GDB identify functions such as `kernel_start`. The historical file named `kernel.bin` was also an ELF executable; its extension did not make it a raw binary.

Expect `ELF64`, little-endian data, machine `RISC-V`, and entry point **`0x1000`**. The program headers contain three `LOAD` segments, with `R E`, `R` and `RW` permissions. Sizes vary with compiler and demo selection. See the [recorded ELF headers](evidence/elf.txt).

| Section | Contents |
| --- | --- |
| `.text` | Instructions, with startup code in `.text.init` |
| `.rodata` | Constants and strings |
| `.data` | Variables with initial values stored in the executable |
| `.bss` | Zero-initialized variables and reserved stacks |

`.bss` occupies memory without storing all its zeros in the file, contributing to the difference between `FileSiz` and `MemSiz`.

[kernel.lds](../src/kernel.lds) defines `ENTRY(entry)`, uses the Makefile's `KERNEL_BASE`, and retains `.text.init` with `KEEP`. Segment permissions organize the ELF; they do not isolate tasks. The kernel does not configure paging or separate process address spaces.

## 2. Prepare C in `entry`

Read [crt0.S](../src/crt0.S) in this order:

| Step | Instruction or symbol | Purpose |
| --- | --- | --- |
| Disable interrupts | `csrw mie, zero`; `csrw mstatus, zero` | Prevent traps during setup |
| Select hart 0 | Read `mhartid` | Other harts enter a wait loop |
| Set the global pointer | `la gp, __global_pointer$` | Prepare global-data addressing |
| Set the stack pointer | `la sp, stacks_end` | Start at the top of the boot stack |
| Clear `.bss` | Loop from `__bss_start` to `__bss_end` | Store zeros eight bytes at a time |
| Enter C | `call kernel_start` | Initialize devices and run the demo |

The boot stack reserves **16 KiB**, grows toward lower addresses, and is aligned to 16 bytes as required by the calling convention. It lies in `.bss`, but the clearing loop runs before any C call uses it. See the [RISC-V calling convention](https://riscv-non-isa.github.io/riscv-elf-psabi-doc/).

The `gp` initialization uses `.option norelax`: this prevents an optimization that would assume `gp` was already valid. The Makefile also uses `-mno-relax`. If `kernel_start` returns, startup code remains in a `wfi` loop.

## 3. Initialize devices and tasks

[kernel_start](../src/start.c) initializes UART, clears the screen and selects the compiled demo. For the default process demo, [test_processus](../src/test_student.c) then:

1. Calls `init_proc()` to prepare the task table and `idle`.
2. Installs `mon_traitant` in `mtvec`.
3. Creates `proc1`, `proc2` and `proc3`.
4. Programs the timer and enables interrupts.
5. Runs `idle`, which waits while timer interrupts schedule tasks.

The table must be ready before interrupts can invoke the scheduler. Calling `init_proc` again would erase the tasks already created.

Everything runs in **machine mode**, or M-mode, including the three task functions. There is no transition to an isolated user program.

## 4. Follow the boot in GDB

In terminal 1:

```bash
sh scripts/course-env.sh make debug
```

QEMU starts paused because of `-S`. In terminal 2, from the same directory:

```bash
gdb-multiarch build/cep/kernel.elf
```

At the GDB prompt:

```gdb
target remote localhost:1234
x/8i $pc
info registers pc sp gp
break kernel_start
continue
info registers pc sp gp
print/x &stacks
print/x &stacks_end
print ((unsigned long)$sp % 16)
```

This CEP machine starts paused **at `entry`, address `0x1000`**. Inspect it directly; setting a breakpoint at that same instruction and continuing would skip the initial stop, with no second boot to hit it.

At `kernel_start`, `sp` and `gp` are initialized. The C function's prologue may already have moved `sp` below `stacks_end`. The final expression should return **zero**, confirming 16-byte alignment.

The [recorded GDB session](evidence/boot-gdb.txt) uses the same `-S` behavior with a local Unix socket and no display. Addresses can change after rebuilding. `stepi` executes one instruction; `continue` resumes normal execution. Finish by closing QEMU and quitting GDB.

At `-O2`, some C variables may be optimized out. For a study session, use `OPT=-O0` consistently when rebuilding and launching.

ELF inspection checks the executable layout; GDB checks entry into C and register state. The current startup adds explicit `.bss` clearing, `gp` setup and segment separation to the original structure; the [inventory](01-inventory.md) records the changes.

Continue with [timer interrupts](06-clock-interrupts.md) or [processes](07-processes.md).
