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
    printf("[idle] pid = %i\n", mon_pid());
    for (;;) {
        enable_it();
        hlt();
    }
}

void kernel_start(void)
{
    disable_it();
    init_uart();
    efface_ecran();
    printf("RISC-V kernel\n");

    /* Set up every task before the first timer interrupt can run. */
    init_proc();
    init_traitant_timer(mon_traitant);
    cree_processus(proc1, "proc1");
    cree_processus(proc2, "proc2");
    cree_processus(proc3, "proc3");
    enable_timer();
    idle();
}
