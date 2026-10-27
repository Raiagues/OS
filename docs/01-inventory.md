# 1. Inventory and original history

The graphical console, UART transmission, processes, scheduler and clock belong to the original project. The final 2023 boot called `test_processus()`, while `test_console()` and `test_timer()` remained in the sources. Running the default demo therefore did not expose every implemented operation.

This audit covered the five local course repositories, uncommitted changes, local branches, remote-tracking references and reflogs: **62 distinct commits and 151 distinct C, header and assembly file versions**. No functional keyboard, UART input or shell subsystem was found. That conclusion applies to the supplied material, not to copies that may exist elsewhere.

## Implemented features

“Original” identifies evidence in the 2023 sources. “Current” describes this repository after recovery fixes. The last column explains how to check a feature; a screenshot alone cannot establish internal kernel behavior.

| Feature | Original evidence | Current implementation | Appropriate verification |
| --- | --- | --- | --- |
| Pixel writes | `pixel()` in `console.c`; console work includes `41c8046` | Preserved, with coordinate bounds checks | Run the pixel demo and inspect video memory |
| 8 × 8 characters | `ecrit_car()` reads the supplied `font8x8_basic` bitmap | Preserved | Compare a character's bitmap with its rendered pixels |
| Cursor positioning and erasure | `place_curseur()`, `efface_position()`, `efface_char()` | Preserved; corrected an extra row being cleared | Follow cursor coordinates and inspect the affected area |
| Newline, tab, carriage return, backspace | `traite_car()` handles `\n`, `\t`, `\r`, `\b` | Preserved; tab bounds corrected | Trace known input sequences and resulting cursor positions |
| Screen clearing | `efface_ecran()` and the `\f` case in `traite_car()` | Preserved | Inspect clearing and cursor reset; `\f` existed but was **not** part of the final original `test_console()` sequence |
| Scrolling | `defilement()` uses `memmove()` | Preserved; movement bounds and last-row clearing corrected | Write more lines than fit and inspect the remaining text |
| Serial transmission | `init_uart()`, `traite_car_uart()`; `4dddfe8`, 2023-10-29 | CEP SiFive UART preserved; explicit initialization, `volatile` access and transmitter readiness checks | Capture QEMU's UART output |
| Mirrored console output | `console_putbytes()` calls both display and UART character routines | Preserved in the CEP backend | Compare the display and serial log from one execution |
| Timer interrupts | `traitant.S`, `init_traitant_timer()`, `enable_timer()`, `trap_handler()`; work starts in `4dddfe8` | Preserved; initialization order and RV64 cause handling corrected | Inspect ticks and CSRs with GDB; observe clock progression |
| On-screen clock | `HH:MM:SS`, `nbr_secondes()`, `ecrit_car_en_haut()` | Preserved; ticks drive the once-per-second display update | Observe the clock at successive times |
| Process creation and identity | `cree_processus()`, `mon_pid()`, `mon_nom()`; `0df81eb`, 2023-11-26 | Preserved: `idle` and three tasks | Check task names, PIDs and table capacity |
| Assembly context switch | `ctx_sw()` in `ctxt.S`; `1b2c495`, 2023-11-26 | Preserved; context includes `mstatus`, stacks are aligned | Check resumed registers and timer preemption without voluntary yielding |
| Round-robin scheduling | `ordonnance()` selects ready tasks and calls `ctx_sw()` | Preserved; shared updates protected against interrupts | Inspect interleaved execution and the preemption test |
| Sleep and wake-up | `dors()`, `hour_wake_up`; `663e39f`, 2023-12-02 | Preserved, including `WAIT` → `RUNNABLE` | Compare timestamps around the original 2-, 3- and 4-second sleeps |
| Process-state panel | `afficher_etats()` and drawing helpers; `71e8380`, 2023-12-03 | Preserved; unsafe formatting buffer removed | Relate displayed digits to the process table |
| Explicit termination | `fin_processus()` sets `DIE` and schedules another task | Preserved | Confirm that a terminated task never resumes |
| Termination on function return | `proc_launcher()` and initial context; `2a88ec3`, 2023-12-14 | Preserved | Follow the return path into `fin_processus()` |
| Keyboard / UART reception | A commit mentions “clavier”, but no input-processing path was found | Not implemented in the audited material | Read the historical evidence below |
| External interrupts / PLIC | Hardware constants and handler sketches | No complete input-device handler; unexpected traps produce `PANIC` | Inspect the handler and device accesses, not just register definitions |

Current sources: [console.c](../src/console.c), [uart.c](../src/uart.c), [time.c](../src/time.c), [processus.c](../src/processus.c), [ctxt.S](../src/ctxt.S), [test_student.c](../src/test_student.c). Commands and observable results are in [Reproduce and validate](08-run-and-verify.md).

## The keyboard commit

Commit **`6e9012789b59373e0791bb684b552eb08ed52580`**, dated 2023-12-14, is titled **“Inclusion gestion des entrées du clavier”**. Its changes concern scheduling, the demo and trap dispatch. The external-interrupt handler is:

```c
void handle_external_interrupt(void)
{
    printf("A faire...");
    csr_clear(mie, MIP_MEIP);
    csr_set(mie, MIP_MEIP);
}
```

`A faire...` means “to do”. This prints a message and toggles an enable bit. It does not identify an interrupt source through the PLIC, read a receive FIFO or deliver input to a program.

Commits `c2d3254` and `c9f3391` reorganize this path and add diagnostics. The final commit, **`3a1eac9`**, removes the `handle_external_interrupt()` and `exception_handle()` sketches, retaining an external-interrupt message. The recovered version reports an unexpected trap explicitly.

`UART_REG_RXFIFO`, `UART_REG_RXCTRL`, `UART_RXEN` and `PLIC_IRQ_CLAIM` in [cep_platform.h](../src/cep_platform.h) came from the supplied hardware map. Their presence does not establish a driver. **Keyboard work was named in the history, but no functional keyboard implementation was found. UART transmission is implemented; reception is a separate path.**

## Repositories examined

| Original directory | Local HEAD | Date |
| --- | --- | --- |
| `ProjetPCSEF_rodriger` | `79474f6` | 2023-11-30 |
| `TP5/ProjetPCSEF_rodriger` | `15d388f` | 2023-12-02 |
| `TP5/PIPELINE CORRETA/ProjetPCSEF_rodriger` | `cdfd2c6` | 2023-12-03 |
| `TP5/OUTRA PIPELINE DE CERTO PFVR EU TE IMPLORO/ProjetPCSEF_rodriger` | `e6d6689` | 2023-12-05 |
| `final/ProjetPCSEF_rodriger` | `3a1eac9` | 2023-12-14 |

The final repository contains 59 of the 62 commits. The other copies contribute `79474f6`, `cdfd2c6` and `e6d6689`; their differences were also examined. Every `origin/fromProf` reference points to `352e9f4`, dated 2023-09-24. Reflogs added no distinct commits beyond those reachable from references.

Uncommitted work was examined separately, including scheduler experiments in intermediate copies and comments in the final `console.c` and `console.h`. Comparing HEAD commits alone would have missed those changes.

## Earlier names and experiments

Some differences reflect the original development process rather than lost features:

- `fact(5)` was a starter recursion/debugging exercise. An early `main.c` held console code later organized into `console.c` and tests.
- `define_curseur()` and `traite()` were earlier cursor helpers. `init_processus()`, `mon_state()` and `get_proc_actif()` were table-management variants; the final version uses `init_proc()`, `actif`, identity functions and the state panel.
- `ordonnance2()`, `mon_etat()` and `affiche_etats()` occur in experimental blocks, some commented out. They do not constitute an additional independent process subsystem.
- `place_curseur_state()` and `traite_car_state()` supported the state panel. Its current implementation avoids the unsafe intermediate buffer. `interrupt_handle()` became part of the `trap_handler()` path; timer support remains.

No implementations of a shell, filesystem, system calls, isolated user processes or networking were found. The static table holds four tasks including `idle`; the final version does not fully recycle terminated slots.

## Recovery changes, separate from the 2023 work

| Change | Reason |
| --- | --- |
| Explicit BSS clearing and boot setup | Reproducible initialization |
| Initialize the table before enabling the timer; remove the demo's second `init_proc()` call | Avoid an interrupt before setup and avoid erasing newly created tasks |
| Correct table bounds, stack alignment and name lengths | Keep writes within allocated storage |
| Preserve `mstatus`; protect scheduler updates; handle RV64 `mcause` | Consistent resumption, preemption and trap dispatch |
| Correct scrolling, clearing, tab and `snprintf()` bounds | Prevent invalid positions and buffer overflows |
| Add the `virt` backend with an NS16550A UART | Run the kernel with standard QEMU; this is a later adaptation, not the original CEP display |
| Add demo selection, captures and automated checks | Make existing operations easier to reproduce and inspect |

CEP remains the primary graphical platform. The teaching demos call project primitives; their output is not evidence of a grade or acceptance by the original course evaluator.

## Rechecking the history locally

These commands run from `OS` in the workspace that still contains the original repositories. A clone of this repository alone does not include those intermediate repositories.

```bash
git -C ../final/ProjetPCSEF_rodriger log --all --oneline
git -C ../final/ProjetPCSEF_rodriger show 6e90127 -- src/time.c
git -C ../final/ProjetPCSEF_rodriger show 3a1eac9 -- src/time.c
git -C ../final/ProjetPCSEF_rodriger show 4dddfe8 -- src/uart.c src/uart.h
git -C ../final/ProjetPCSEF_rodriger show 2a88ec3 -- src/processus.c src/ctxt.S
git -C ../final/ProjetPCSEF_rodriger status --short
```

Sébastien Viardot supplied the starting point `352e9f4`: boot code, linker setup, platform definitions, bitmap font and support routines. Emilly Raiane Rodrigues's student work added the console, UART, timer/traps and task management throughout the history. Third-party notices remain in their source files.

[Back to README](../README.md) · [Reproduce and validate](08-run-and-verify.md)
