OUTPUT_ARCH(riscv)
ENTRY(entry)
PHDRS
{
    text PT_LOAD FLAGS(5);
    rodata PT_LOAD FLAGS(4);
    data PT_LOAD FLAGS(6);
}
/* CEP executable base, supplied by this folder's Makefile. */
SECTIONS
{
    . = KERNEL_BASE;
    .text : ALIGN(16) {
        KEEP(*(.text.init))
        *(.text .text.*)
    } :text
    .rodata : ALIGN(16) { *(.rodata .rodata.*) } :rodata
    .data : ALIGN(16) {
        __global_pointer$ = . + 0x800;
        *(.sdata .sdata.* .data .data.*)
    } :data
    .bss (NOLOAD) : ALIGN(16) {
        __bss_start = .;
        *(.sbss .sbss.* .bss .bss.* COMMON)
        . = ALIGN(16);
        __bss_end = .;
    } :data
    _end = .;
    /DISCARD/ : { *(.comment .note .note.* .eh_frame .eh_frame.*) }
}
