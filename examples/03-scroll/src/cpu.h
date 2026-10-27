#ifndef CPU_H
#define CPU_H
#define MSTATUS_MIE 0x8

static inline void hlt(void)
{
    __asm__ volatile("wfi" ::: "memory");
}

static inline void enable_it(void)
{
    __asm__ volatile("csrs mstatus, %0" :: "i"(MSTATUS_MIE) : "memory");
}
#endif
