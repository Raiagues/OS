#ifndef __CPU_H__
#define __CPU_H__
#include <cep_platform.h>
/* Met le processeur en pause en attente d'une interruption */
__inline__ static void hlt(void)
{
   __asm__ __volatile__("wfi":::"memory");
}

/* Autorise les interruptions au niveau du processeur */
__inline__ static void enable_it(void)
{
   __asm__ __volatile__("csrs mstatus, %0"::"i"(MSTATUS_MIE):"memory");
}

/* Interdit les interruptions au niveau du processeur */
__inline__ static void disable_it(void)
{
   __asm__ __volatile__("csrc mstatus, %0"::"i"(MSTATUS_MIE):"memory");
}

#endif
