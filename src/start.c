#include "test_student.h"
#include "demos.h"
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
    printf("OS / RISC-V RV64 / QEMU CEP\n");
#else
    printf("OS / RISC-V RV64 / QEMU virt\n");
#endif
#if defined(KERNEL_SELFTEST) || defined(DEMO_PROCESSES)
    printf("Timer: 20 Hz | processes: idle + 3 | mode: M\n");
    test_processus();
#else
    demo_run();
#endif
    for (;;) hlt();
}
