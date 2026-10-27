#include "console.h"
#include "processus.h"
#include "time.h"
#include "uart.h"

void proc1(void)
{
    for (int i = 0; i < 2; ++i) {
        printf("[temps = %u s] processus %s pid = %i\n", nbr_secondes(), mon_nom(), mon_pid());
        dors(2);
    }
    /* Returning reaches fin_processus() through proc_launcher(). */
}

void proc2(void)
{
    for (int i = 0; i < 2; ++i) {
        printf("[temps = %u s] processus %s pid = %i\n", nbr_secondes(), mon_nom(), mon_pid());
        dors(3);
    }
    fin_processus();
}

void proc3(void)
{
    for (int i = 0; i < 2; ++i) {
        printf("[temps = %u s] processus %s pid = %i\n", nbr_secondes(), mon_nom(), mon_pid());
        dors(4);
    }
    /* This task also terminates by returning. */
}

void idle(void)
{
    int announced = 0;
    printf("[idle] pid = %i\n", mon_pid());
    for (;;) {
        int finished = 1;
        for (int i = 1; i < n_proc; ++i) {
            if (table_des_processus[i].state != DIE) { finished = 0; }
        }
        if (finished && !announced && n_proc == N) {
            printf("[OS] DEMO COMPLETE: proc1, proc2, proc3 finished; idle running.\n");
            announced = 1;
        }
        enable_it();
        hlt();
    }
}

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
    printf("Timer: 20 Hz | processes: idle + 3 | mode: M\n");

    /* Set up every task before the first timer interrupt can run. */
    init_proc();
    init_traitant_timer(mon_traitant);
    cree_processus(proc1, "proc1");
    cree_processus(proc2, "proc2");
    cree_processus(proc3, "proc3");
    enable_timer();
    idle();
}
