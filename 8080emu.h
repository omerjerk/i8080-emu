#include <inttypes.h>

#ifndef EMU_H
#define EMU_H

typedef struct ConditionCodes {
    uint8_t    z:1;
    uint8_t    s:1;
    uint8_t    p:1;
    uint8_t    cy:1;
    uint8_t    ac:1;
    uint8_t    pad:3;
} ConditionCodes;

// Space invaders I/O ports
typedef struct Ports {
    uint8_t     read1;
    uint8_t     read2;
    uint8_t     shift1;
    uint8_t     shift0;

    uint8_t     write2; // shift amount (bits 0,1,2)
    uint8_t     write4; // shift data
} Ports;

typedef struct State8080 {
    uint8_t    a;
    uint8_t    b;
    uint8_t    c;
    uint8_t    d;
    uint8_t    e;
    uint8_t    h;
    uint8_t    l;
    uint16_t   sp;
    uint16_t   pc;
    uint8_t    *memory;
    struct     ConditionCodes      cc;
	struct      Ports              port;
    uint8_t    int_enable;
} State8080;

int disassemble8080Op(unsigned char *code, int pc);
int emulate8080(State8080* state);
void generateInterrupt(State8080 *state, int interrupt_num);

#endif