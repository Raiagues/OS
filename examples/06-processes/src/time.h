#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <cpu.h>
#include "console.h"

#define ITFREQ 20
#define MIP_MTIP (1UL << IRQ_M_TMR)

extern void mon_traitant(void);
void handle_time_interrupt(void);
void trap_handler(uint64_t mcause);
void init_traitant_timer(void (*traitant)(void));
uint32_t nbr_secondes(void);
void enable_timer(void);

/* Accès RV64 aux CSR ; les barrières empêchent le déplacement des accès mémoire. */
#define csr_read(csr) ({ \
    unsigned long value; \
    __asm__ volatile ("csrr %0, " #csr : "=r" (value) : : "memory"); \
    value; \
})
#define csr_write(csr, val) do { \
    unsigned long value = (unsigned long)(val); \
    __asm__ volatile ("csrw " #csr ", %0" : : "rK" (value) : "memory"); \
} while (0)
#define csr_set(csr, val) do { \
    unsigned long value = (unsigned long)(val); \
    __asm__ volatile ("csrs " #csr ", %0" : : "rK" (value) : "memory"); \
} while (0)
#define csr_read_clear(csr, val) ({ \
    unsigned long previous; \
    unsigned long value = (unsigned long)(val); \
    __asm__ volatile ("csrrc %0, " #csr ", %1" \
                      : "=r" (previous) : "rK" (value) : "memory"); \
    previous; \
})

#endif
