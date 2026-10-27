#include "test_student.h"
#include "cpu.h"
#include "uart.h"
#include "console.h"
#include "stdio.h"

void kernel_start(void)
{
    disable_it();
    init_uart();
    efface_ecran();
#ifdef PLATFORM_CEP
    printf("OS / RISC-V RV64 / plataforma CEP\n");
#else
    printf("OS / RISC-V RV64 / QEMU virt\n");
#endif
    printf("Timer: 20 Hz | processos: idle + 3 | modo: M\n");
    test_processus();
    for (;;) hlt();
}
