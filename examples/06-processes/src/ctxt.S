
.global ctx_sw
.text
/* void ctx_sw(uint64_t *ctxt_de_sauvegarde, uint64_t *ctxt_a_restaurer)
    ctxt_de_sauvegarde : registre a0
    ctxt_a_restaurer   : registre a1
*/
ctx_sw:
    sd ra,   0*8(a0)
    sd sp,   1*8(a0)
    sd gp,   2*8(a0)
    sd tp,   3*8(a0)
    sd s0,   4*8(a0)
    sd s1,   5*8(a0)
    sd s2,   6*8(a0)
    sd s3,   7*8(a0)
    sd s4,   8*8(a0)
    sd s5,   9*8(a0)
    sd s6,  10*8(a0)
    sd s7,  11*8(a0)
    sd s8,  12*8(a0)
    sd s9,  13*8(a0)
    sd s10, 14*8(a0)
    sd s11, 15*8(a0)
    csrrs s1,mepc,zero
    sd s1, 16*8(a0)
    csrr s1,mstatus
    sd s1, 17*8(a0)

    # MIE reste à zéro pendant le changement de pile et de registres.
    # L'appelant (ou proc_launcher) rétablit ensuite son état d'interruption.
    ld s1, 17*8(a1)
    andi s1, s1, -9
    csrw mstatus,s1
    ld s1, 16*8(a1)
    csrrw zero,mepc,s1
    ld ra,   0*8(a1)
    ld sp,   1*8(a1)
    ld gp,   2*8(a1)
    ld tp,   3*8(a1)
    ld s0,   4*8(a1)
    ld s1,   5*8(a1)
    ld s2,   6*8(a1)
    ld s3,   7*8(a1)
    ld s4,   8*8(a1)
    ld s5,   9*8(a1)
    ld s6,  10*8(a1)
    ld s7,  11*8(a1)
    ld s8,  12*8(a1)
    ld s9,  13*8(a1)
    ld s10, 14*8(a1)
    ld s11, 15*8(a1)
    mv a0,  s1
    ret
