#include "processus.h"
#include "time.h"
#include "console.h"

processus_t table_des_processus[N];
processus_t *actif;
int n_proc;

/* Chaque appelant conserve son état d'interruption sur sa propre pile. */
static unsigned long irq_save(void)
{
    return csr_read_clear(mstatus, MSTATUS_MIE);
}

static void irq_restore(unsigned long flags)
{
    if (flags & MSTATUS_MIE) {
        enable_it();
    } else {
        disable_it();
    }
}

int32_t mon_pid(void) { return actif->pid; }
char *mon_nom(void) { return actif->name; }

int32_t cree_processus(void (*code)(void), char *nom)
{
    unsigned long flags = irq_save();
    if (n_proc >= N || code == 0 || nom == 0) {
        irq_restore(flags);
        return -1;
    }

    processus_t *p = &table_des_processus[n_proc];
    memset(p, 0, sizeof(*p));
    p->pid = n_proc++;
    strncpy(p->name, nom, sizeof(p->name) - 1);
    p->reg[0] = (uint64_t)proc_launcher;
    p->reg[1] = (uint64_t)&p->pille[TAILLE_PILLE];
    p->reg[2] = table_des_processus[0].reg[2];
    p->reg[3] = table_des_processus[0].reg[3];
    p->reg[5] = (uint64_t)code;
    /* Nouvelle tâche en mode machine, MIE=0 jusqu'à proc_launcher(). */
    p->reg[17] = (3UL << 11) | (1UL << 7);
    p->state = RUNNABLE;
    printf("Processus: %s cree avec pid %d\n", p->name, p->pid);
    irq_restore(flags);
    return p->pid;
}

void init_proc(void)
{
    disable_it();
    memset(table_des_processus, 0, sizeof(table_des_processus));
    for (int i = 0; i < N; ++i) {
        table_des_processus[i].pid = i;
        table_des_processus[i].state = DIE;
    }
    actif = &table_des_processus[0];
    strcpy(actif->name, "idle");
    actif->state = RUN;
    /* idle commence sur la pile de démarrage ; ctx_sw sauvegardera son sp. */
    __asm__ volatile ("mv %0, gp" : "=r" (actif->reg[2]));
    __asm__ volatile ("mv %0, tp" : "=r" (actif->reg[3]));
    n_proc = 1;
}

void ordonnance(void)
{
    unsigned long flags = irq_save();
    processus_t *ancien = actif;
    uint32_t maintenant = nbr_secondes();

    for (int i = 1; i < n_proc; ++i) {
        processus_t *p = &table_des_processus[i];
        if (p->state == WAIT && (int32_t)(maintenant - p->hour_wake_up) >= 0) {
            p->state = RUNNABLE;
            printf("Fin du sommeil de %u s du processus %d\n", p->temp_sommeil, p->pid);
        }
    }

    if (ancien->state == RUN) {
        ancien->state = RUNNABLE;
    }
    /* Le tour inclut la tâche actuelle : idle reste disponible si tout dort. */
    for (int distance = 1; distance <= n_proc; ++distance) {
        processus_t *suivant = &table_des_processus[(ancien->pid + distance) % n_proc];
        if (suivant->state == RUNNABLE) {
            suivant->state = RUN;
            actif = suivant;
            if (suivant != ancien) {
                ctx_sw(ancien->reg, suivant->reg);
            }
            break;
        }
    }
    irq_restore(flags);
}

void dors(uint32_t nbr_secs)
{
    unsigned long flags = irq_save();
    if (actif->pid != 0 && nbr_secs != 0) {
        /* Les échéances signées doivent rester dans une demi-plage uint32_t. */
        if (nbr_secs > 0x7fffffffU) {
            nbr_secs = 0x7fffffffU;
        }
        actif->hour_wake_up = nbr_secondes() + nbr_secs;
        actif->temp_sommeil = nbr_secs;
        actif->state = WAIT;
        printf("Debut du sommeil de %u s du processus %d\n", nbr_secs, actif->pid);
    }
    ordonnance();
    irq_restore(flags);
}

_Noreturn void fin_processus(void)
{
    disable_it();
    if (actif->pid != 0) {
        actif->state = DIE;
        printf("Fin du processus %d\n", actif->pid);
        ordonnance();
    }
    /* Une tâche DIE ne doit jamais reprendre, et idle ne peut pas terminer. */
    printf("PANIC: retour d'un processus termine\n");
    for (;;) { hlt(); }
}

void proc_launcher(void (*proc)(void))
{
    /* Une tâche choisie depuis une interruption doit pouvoir être préemptée. */
    enable_it();
    proc();
    fin_processus();
}

static void ecrit_etat(uint32_t colonne, char c)
{
    for (uint32_t y = 0; y < BITS; ++y) {
        for (uint32_t x = 0; x < BITS; ++x) {
            pixel(colonne * BITS + x, y, NOIR);
        }
    }
    ecrit_car(0, colonne, c);
}

void afficher_etats(void)
{
    /* Une case par chiffre, sans sprintf ni dépassement du tampon original. */
    ecrit_etat(0, '[');
    for (int i = 0; i < n_proc; ++i) {
        ecrit_etat((uint32_t)(1 + 2 * i), (char)('0' + table_des_processus[i].state));
        ecrit_etat((uint32_t)(2 + 2 * i), ' ');
    }
    ecrit_etat((uint32_t)(2 * n_proc), ']');
}
