#include "console.h"
#include "processus.h"
#include "time.h"
#include "uart.h"

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
    /* No sleep, yield or wfi: the timer must preempt this loop. */
    while ((uint32_t)(nbr_secondes() - debut) < 2) {
        __asm__ volatile ("" : : : "memory");
    }
    if (!second_started) { selftest_fail("preemption"); }
    preempt_passed = 1;
    printf("SELFTEST: PREEMPT PASS\n");
    /* Return through proc_launcher to exercise implicit termination. */
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
            if (!preempt_passed || !sleep_passed || !explicit_exit) {
                selftest_fail("exit");
            }
            printf("SELFTEST: EXIT PASS\nSELFTEST: ALL PASS\n");
            annonce = 1;
        }
        enable_it();
        hlt();
    }
}

static void run_selftests(void)
{
    /* Prepare the table before the first timer interrupt. */
    init_proc();
    init_traitant_timer(mon_traitant);
    printf("SELFTEST: START\n");
    test_format_bounds();
    cree_processus(proc1, "proc1");
    cree_processus(proc2, "proc2");
    cree_processus(proc3, "proc3");
    enable_timer();
    idle();
}

void kernel_start(void)
{
    disable_it();
    init_uart();
    efface_ecran();
    run_selftests();
    for (;;) hlt();
}
