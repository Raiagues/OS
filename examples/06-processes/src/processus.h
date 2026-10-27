#ifndef PROCESSUS_H
#define PROCESSUS_H

#include <inttypes.h>

/* Quatre tâches au total, dont idle ; chaque pile contient 4096 mots de 64 bits. */
#define N 4
#define TAILLE_PILLE 4096

enum { RUN, RUNNABLE, WAIT, DIE };

typedef struct {
    int pid;
    char name[32];
    int state;
    /* ra, sp, gp, tp, s0..s11, mepc, mstatus (voir ctxt.S). */
    uint64_t reg[18];
    _Alignas(16) uint64_t pille[TAILLE_PILLE];
    uint32_t hour_wake_up;
    uint32_t temp_sommeil;
} processus_t;

extern processus_t table_des_processus[N];
extern processus_t *actif;
extern int n_proc;

void ctx_sw(uint64_t *ancien, uint64_t *nouveau);
/* Initialise au démarrage, avant d'activer le timer ; ne pas appeler d'une tâche. */
void init_proc(void);
/* Retourne -1 si la table est pleine ou les arguments sont invalides. */
int32_t cree_processus(void (*code)(void), char *nom);
void ordonnance(void);
void proc_launcher(void (*proc)(void));
_Noreturn void fin_processus(void);
void dors(uint32_t nbr_secs);
int32_t mon_pid(void);
char *mon_nom(void);
void afficher_etats(void);

#endif
