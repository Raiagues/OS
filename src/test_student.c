#include "test_student.h"
#include "processus.h"
#include "time.h"

void test_console(void)
{
    printf("Truc\nSalut\n1\t2\t3\nAB\bC\nBEEF\rRABBIT\n");
    for (int i = 0; i < 84; ++i) { printf("%d\n", i); }
    printf("Hello A\bBC\tTAB 12345\t678\n9\n");
}

void test_timer(void)
{
    init_proc();
    init_traitant_timer(mon_traitant);
    enable_timer();
}

#ifdef KERNEL_SELFTEST
static volatile unsigned int second_started;
static volatile unsigned int preempt_passed;
static volatile unsigned int sleep_passed;
static volatile unsigned int explicit_exit;

static _Noreturn void selftest_fail(const char *message)
{
    printf("SELFTEST: FAIL %s\n", message);
    disable_it();
    for (;;) { hlt(); }
}

static void test_format_bounds(void)
{
    struct {
        char buffer[4];
        unsigned char canary;
    } guarded = { { 'x', 'x', 'x', 'x' }, 0xa5 };
    char one[2] = { 'x', 'y' };
    int length = snprintf(guarded.buffer, sizeof(guarded.buffer), "%s", "abcdef");
    if (length != 6 || strcmp(guarded.buffer, "abc") != 0 || guarded.canary != 0xa5) {
        selftest_fail("snprintf truncation/canary");
    }
    if (snprintf(0, 0, "%s", "abcdef") != 6) {
        selftest_fail("snprintf size zero");
    }
    if (snprintf(one, 1, "%s", "abcdef") != 6 || one[0] != '\0' || one[1] != 'y') {
        selftest_fail("snprintf size one");
    }
    printf("SELFTEST: FORMAT PASS\n");
}

void proc1(void)
{
    uint32_t debut = nbr_secondes();
    /* Aucun dors(), ordonnance() ou wfi : seul le timer peut donner la main. */
    while ((uint32_t)(nbr_secondes() - debut) < 2) {
        __asm__ volatile ("" : : : "memory");
    }
    if (!second_started) { selftest_fail("preemption"); }
    preempt_passed = 1;
    printf("SELFTEST: PREEMPT PASS\n");
    /* Retour implicite : proc_launcher doit terminer cette tâche. */
}

void proc2(void)
{
    second_started = 1;
    uint32_t debut = nbr_secondes();
    dors(2);
    if ((uint32_t)(nbr_secondes() - debut) < 2) { selftest_fail("sleep"); }
    sleep_passed = 1;
    printf("SELFTEST: SLEEP PASS\n");
}

void proc3(void)
{
    explicit_exit = 1;
    fin_processus();
}
#else
void proc1(void)
{
    for (int i = 0; i < 2; ++i) {
        printf("[temps = %u s] processus %s pid = %i\n", nbr_secondes(), mon_nom(), mon_pid());
        dors(2);
    }
    /* Terminaison implicite par retour de fonction. */
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
}
#endif

void idle(void)
{
    int annonce = 0;
    printf("[idle] pid = %i\n", mon_pid());
    for (;;) {
        int termine = 1;
        for (int i = 1; i < n_proc; ++i) {
            if (table_des_processus[i].state != DIE) { termine = 0; }
        }
        if (termine && !annonce && n_proc == N) {
#ifdef KERNEL_SELFTEST
            if (!preempt_passed || !sleep_passed || !explicit_exit) {
                selftest_fail("exit");
            }
            printf("SELFTEST: EXIT PASS\nSELFTEST: ALL PASS\n");
#else
            printf("[OS] DEMO COMPLETE: proc1, proc2, proc3 finished; idle running.\n");
#endif
            annonce = 1;
        }
        enable_it();
        hlt();
    }
}

void test_processus(void)
{
    /* Construire toute la table avant la première interruption. */
    init_proc();
    init_traitant_timer(mon_traitant);
#ifdef KERNEL_SELFTEST
    printf("SELFTEST: START\n");
    test_format_bounds();
#endif
    cree_processus(proc1, "proc1");
    cree_processus(proc2, "proc2");
    cree_processus(proc3, "proc3");
    enable_timer();
    idle();
}
