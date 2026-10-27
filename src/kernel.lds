OUTPUT_ARCH(riscv)
ENTRY(entry)
/* Passed by the Makefile: virt RAM = 0x80000000, original CEP = 0x1000. */
SECTIONS
{
    . = KERNEL_BASE;
    .text : ALIGN(16) {
        KEEP(*(.text.init))
        *(.text .text.*)
    }
    .rodata : ALIGN(16) { *(.rodata .rodata.*) }
    .data : ALIGN(16) {
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .data .data.*)
    }
    .bss (NOLOAD) : ALIGN(16) {
        __bss_start = .;
        *(.sbss .sbss.* .bss .bss.* COMMON)
        . = ALIGN(16);
        __bss_end = .;
    }
    _end = .;
    /DISCARD/ : { *(.comment .note .note.* .eh_frame .eh_frame.*) }
}
