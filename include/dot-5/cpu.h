#ifndef _D5_CPU_H_
#define _D5_CPU_H_

#include "memory_map.h"

typedef enum {
    INC = 0x0,
    DEC = 0x1,
    LDA_I = 0x2,
    LDA_Z = 0x3,
    STA = 0x4,
    JMP = 0x5,
    BEQ = 0x6,
    BNE = 0x7,
    ADD_I = 0x8,
    ADD_Z = 0x9,
    SUB_I = 0xa,
    SUB_Z = 0xb,
    ORA_I = 0xc,
    ORA_Z = 0xd,
    AND_I = 0xe,
    AND_Z = 0xf,
} ISA;

extern void cpu_reset();
extern byte cpu_step();

#endif