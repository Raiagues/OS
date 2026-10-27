    .section .text.init,"ax",@progbits
    .balign 16
    .global entry
entry:
    # One hart only. Keep interrupts off until the kernel is initialized.
    csrw mie, zero
    csrw mstatus, zero
    csrr t0, mhartid
    bnez t0, .Lhalt
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop
    la sp, stacks_end
    la t0, __bss_start
    la t1, __bss_end
.Lzero:
    bgeu t0, t1, .Lstart
    sd zero, 0(t0)
    addi t0, t0, 8
    j .Lzero
.Lstart:
    call kernel_start
.Lhalt:
    wfi
    j .Lhalt

    .section .bss.stack,"aw",@nobits
    .balign 16
    .global stacks, stacks_end
stacks:
    .skip 16384
stacks_end:
