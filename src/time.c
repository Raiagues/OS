#include "time.h"
#include "processus.h"

/* Le compteur évolue dans le gestionnaire d'interruption, même en boucle active. */
static volatile uint64_t ticks;

static void programme_timer(void)
{
    *((volatile uint64_t *)CLINT_TIMER_CMP) =
        *((volatile uint64_t *)CLINT_TIMER) + TIMER_FREQ / ITFREQ;
}

void handle_time_interrupt(void)
{
    programme_timer();
    ++ticks;
    if (ticks % ITFREQ == 0) {
        uint32_t secondes = nbr_secondes();
        char timer[9];
        snprintf(timer, sizeof(timer), "%02u:%02u:%02u",
                 (secondes / 3600) % 100, (secondes / 60) % 60, secondes % 60);
        ecrit_car_en_haut(timer, 8);
        afficher_etats();
    }
    ordonnance();
}

void trap_handler(uint64_t mcause)
{
    if ((mcause & (1ULL << 63)) != 0 &&
        (mcause & ~(1ULL << 63)) == IRQ_M_TMR) {
        handle_time_interrupt();
        return;
    }
    disable_it();
    csr_write(mie, 0);
    printf("PANIC: mcause=0x%lx mepc=0x%lx mtval=0x%lx\n",
           (unsigned long)mcause, csr_read(mepc), csr_read(mtval));
    for (;;) { hlt(); }
}

void init_traitant_timer(void (*traitant)(void))
{
    csr_write(mtvec, traitant);
}

uint32_t nbr_secondes(void)
{
    return (uint32_t)(ticks / ITFREQ);
}

void enable_timer(void)
{
    /* Armer avant MIE évite une interruption immédiate avec mtimecmp=0. */
    programme_timer();
    csr_set(mie, MIP_MTIP);
    enable_it();
}
