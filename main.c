#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <gtk/gtk.h>

#include "main.h"
#include "8080emu.h"

void unimplementedInstruction(State8080* state) {
    // pc will have advanced one, so undo that
    printf ("Error: Unimplemented instruction\n");
    exit(1);
}

int parity(int x, int size) {
    int i;
    int p = 0;
    x = (x & ((1<<size)-1));
    for (i=0; i<size; i++)
    {
        if (x & 0x1) p++;
        x = x >> 1;
    }
    return (0 == (p & 0x1));
}

uint8_t machineIN(State8080* state, uint8_t port)
{
    uint8_t a;
    switch(port)
    {
        case 1:
            a = state->port.read1;
            break;
        case 2:
            a = state->port.read2;
            break;
        case 3:
        {
            uint16_t v = (state->port.shift1<<8) | state->port.shift0;
            a = ((v >> (8 - state->port.write2)) & 0xff);
            break;
        }
        default:
            unimplementedInstruction(state);
            break;
    }
    return a;
}

void machineOUT(State8080* state, uint8_t port)
{
    switch(port)
    {
        case 2:
            state->port.write2 = state->a & 0x7;
            break;
        case 4:
            state->port.shift0 = state->port.shift1;
            state->port.shift1 = state->a;
            break;
    }
}


int emulate8080(State8080* state) {
    unsigned char *opcode = &state->memory[state->pc];

	disassemble8080Op(opcode, state->pc);

    state->pc += 1;

    switch(*opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LXI B, word
            state->c = opcode[1];
            state->b = opcode[2];
            state->pc += 2; // Advance 2 more bytes
            break;
        case 0x02: // STAX B
        {
            uint16_t pair = (state->b << 8) | (state->c);
            state->memory[pair] = state->a;
            break;
        }
        case 0x03: // INX B
        {
            uint16_t pair = (state->b << 8) | (state->c);
            pair = pair + 1;
            state->b = (pair & 0xff00) >> 8;
            state->c = pair & 0xff;
            break;
        }
        case 0x04: // INR B
            state->b = state->b + 1;
            state->cc.z = ((state->b) == 0);
            state->cc.s = ((state->b & 0x80) == 0x80);
            state->cc.p = parity(state->b, 8);
            state->cc.ac = ((state->b & 0xf) == 0xf);
            break;
        case 0x05: // DCR B
            state->b = state->b - 1;
            state->cc.z = ((state->b) == 0);
            state->cc.s = ((state->b & 0x80) == 0x80);
            state->cc.p = parity(state->b, 8);
            state->cc.ac = ((state->b & 0xf) == 0x0);
            break;
        case 0x06: // MVI B, byte
            state->b = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x07: // RLC
        {
            uint8_t x = state->a;
            state-> a = ((x << 1) | (x & 0x80) >> 7);
            state->cc.cy = ((x & 0x80) == 0x80);
            break;
        }
        case 0x09: // DAD B
        {
            uint32_t pairHL = (state->h << 8) | (state->l);
            uint32_t pairBC = (state->b << 8) | (state->c);
            pairHL = pairHL + pairBC;
            state->cc.cy = (pairHL > 0xff);
            state->h = (pairHL & 0xff00) >> 8;
            state->l = pairHL & 0xff;
            break;
        }
        case 0x0A: // LDAX B
        {
            uint16_t pair = (state->b << 8) | (state->c);
            state->a = state->memory[pair];
            break;
        }
        case 0x0B: // DCX B
        {
            uint16_t pair = (state->b << 8) | (state->c);
            pair = pair - 1;
            state->b = (pair & 0xff00) >> 8;
            state->c = pair & 0xff;
            break;
        }
        case 0x0C: // INR C
            state->c = state->c + 1;
            state->cc.z = ((state->c) == 0);
            state->cc.s = ((state->c & 0x80) == 0x80);
            state->cc.p = parity(state->c, 8);
            state->cc.ac = ((state->c & 0xf) == 0xf);
            break;
        case 0x0D: // DCR C
            state->c = state->c - 1;
            state->cc.z = ((state->c) == 0);
            state->cc.s = ((state->c & 0x80) == 0x80);
            state->cc.p = parity(state->c, 8);
            state->cc.ac = ((state->c & 0xf) == 0x0);
            break;
        case 0x0E: // MVI C, byte
            state->c = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x0F: // RRC
        {
            uint8_t x = state->a;
            state-> a = ((x & 1) << 7 | (x >> 1));
            state->cc.cy = ((x & 1) == 1);
            break;
        }
        case 0x11: // LXI D, word
            state->e = opcode[1];
            state->d = opcode[2];
            state->pc += 2; // Advance 2 more bytes
            break;
        case 0x12: // STAX D
        {
            uint16_t pair = (state->d << 8) | (state->e);
            state->memory[pair] = state->a;
            break;
        }
        case 0x13: // INX D
        {
            uint16_t pair = (state->d << 8) | (state->e);
            pair = pair + 1;
            state->d = (pair & 0xff00) >> 8;
            state->e = pair & 0xff;
            break;
        }
        case 0x14: // INR D
            state->d = state->d + 1;
            state->cc.z = ((state->d) == 0);
            state->cc.s = ((state->d & 0x80) == 0x80);
            state->cc.p = parity(state->d, 8);
            state->cc.ac = ((state->d & 0xf) == 0xf);
            break;
        case 0x15: // DCR D
            state->d = state->d - 1;
            state->cc.z = ((state->d) == 0);
            state->cc.s = ((state->d & 0x80) == 0x80);
            state->cc.p = parity(state->d, 8);
            state->cc.ac = ((state->d & 0xf) == 0x0);
            break;
        case 0x16: // MVI D, byte
            state->d = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x17: // RAL
        {
            uint8_t x = state->a;
            state-> a = ((x << 1) | state->cc.cy);
            state->cc.cy = ((x & 0x80) == 0x80);
            break;
        }
        case 0x19: // DAD D
        {
            uint32_t pairHL = (state->h << 8) | (state->l);
            uint32_t pairDE = (state->d << 8) | (state->e);
            pairHL = pairHL + pairDE;
            state->cc.cy = (pairHL > 0xff);
            state->h = (pairHL & 0xff00) >> 8;
            state->l = pairHL & 0xff;
            break;
        }
        case 0x1A: // LDAX D
        {
            uint16_t pair = (state->d << 8) | (state->e);
            state->a = state->memory[pair];
            break;
        }
        case 0x1B: // DCX D
        {
            uint16_t pair = (state->d << 8) | (state->e);
            pair = pair - 1;
            state->b = (pair & 0xff00) >> 8;
            state->e = pair & 0xff;
            break;
        }
        case 0x1C: // INR E
            state->e = state->e + 1;
            state->cc.z = ((state->e) == 0);
            state->cc.s = ((state->e & 0x80) == 0x80);
            state->cc.p = parity(state->e, 8);
            state->cc.ac = ((state->e & 0xf) == 0xf);
            break;
        case 0x1D: // DCR E
            state->e = state->e - 1;
            state->cc.z = ((state->e) == 0);
            state->cc.s = ((state->e & 0x80) == 0x80);
            state->cc.p = parity(state->e, 8);
            state->cc.ac = ((state->e & 0xf) == 0x0);
            break;
        case 0x1E: // MVI E, byte
            state->e = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x1F: // RAR
        {
            uint8_t x = state->a;
            state-> a = ((state->cc.cy << 7) | (x >> 1));
            state->cc.cy = ((x & 1) == 1);
            break;
        }
        case 0x21: // LXI H, word
            state->l = opcode[1];
            state->h = opcode[2];
            state->pc += 2; // Advance 2 more bytes
            break;
        case 0x23: // INX H
        {
            uint16_t pair = (state->h << 8) | (state->l);
            pair = pair + 1;
            state->h = (pair & 0xff00) >> 8;
            state->l = pair & 0xff;
            break;
        }
        case 0x24: // INR H
            state->h = state->h + 1;
            state->cc.z = ((state->h) == 0);
            state->cc.s = ((state->h & 0x80) == 0x80);
            state->cc.p = parity(state->h, 8);
            state->cc.ac = ((state->h & 0xf) == 0xf);
            break;
        case 0x25: // DCR H
            state->h = state->h - 1;
            state->cc.z = ((state->h) == 0);
            state->cc.s = ((state->h & 0x80) == 0x80);
            state->cc.p = parity(state->h, 8);
            state->cc.ac = ((state->h & 0xf) == 0x0);
            break;
        case 0x26: // MVI H, byte
            state->h = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x29: // DAD H
        {
            uint32_t pairHL = (state->h << 8) | (state->l);
            pairHL = pairHL + pairHL;
            state->cc.cy = (pairHL > 0xff);
            state->h = (pairHL & 0xff00) >> 8;
            state->l = pairHL & 0xff;
            break;
        }
        case 0x2B: // DCX H
        {
            uint16_t pair = (state->h << 8) | (state->l);
            pair = pair - 1;
            state->b = (pair & 0xff00) >> 8;
            state->l = pair & 0xff;
            break;
        }
        case 0x2C: // INR L
            state->l = state->l + 1;
            state->cc.z = ((state->l) == 0);
            state->cc.s = ((state->l & 0x80) == 0x80);
            state->cc.p = parity(state->l, 8);
            state->cc.ac = ((state->l & 0xf) == 0xf);
            break;
        case 0x2D: // DCR L
            state->l = state->l - 1;
            state->cc.z = ((state->l) == 0);
            state->cc.s = ((state->l & 0x80) == 0x80);
            state->cc.p = parity(state->l, 8);
            state->cc.ac = ((state->l & 0xf) == 0x0);
            break;
        case 0x2E: // MVI l, byte
            state->l = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x2F: // CMA (not)
            state->a = ~state->a;
            break;
        case 0x31: // LXI SP, word
            state->sp = (opcode[2] << 8) | (opcode[1]);
            state->pc += 2; // Advance 2 more bytes
            break;
        case 0x32: // STA adr
        {
            uint16_t adr = (opcode[2] << 8) | (opcode[1]);
            state->memory[adr] = state->a;
            state->pc += 2;
            break;
        }
        case 0x33: // INX M
        {
            uint16_t pair = (state->h << 8) | (state->l);
            pair = pair + 1;
            state->h = (pair & 0xff00) >> 8;
            state->l = pair & 0xff;
            break;
        }
        case 0x34: // INR M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->memory[offset] + 1;
            state->cc.z = ((state->memory[offset]) == 0);
            state->cc.s = ((state->memory[offset] & 0x80) == 0x80);
            state->cc.p = parity(state->memory[offset], 8);
            state->cc.ac = ((state->memory[offset] & 0xf) == 0xf);
            break;
        }
        case 0x35: // DCR M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->memory[offset] - 1;
            state->cc.z = ((state->memory[offset]) == 0);
            state->cc.s = ((state->memory[offset] & 0x80) == 0x80);
            state->cc.p = parity(state->memory[offset], 8);
            state->cc.ac = ((state->memory[offset] & 0xf) == 0x0);
            break;
        }
        case 0x36: // MVI M, byte
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = opcode[1];
            state->pc++; // for the data byte
            break;
        }
        case 0x37: // STC
            state->cc.cy = 1;
            break;
        case 0x39: // DAD SP
        {
            uint32_t pairHL = (state->h << 8) | (state->l);
            pairHL = pairHL + (uint32_t) state->sp;
            state->cc.cy = (pairHL > 0xff);
            state->h = (pairHL & 0xff00) >> 8;
            state->l = pairHL & 0xff;
            break;
        }
        case 0x3A: // LDA adr
        {
            uint16_t adr = (opcode[2] << 8) | (opcode[1]);
            state->a = state->memory[adr];
            state->pc += 2;
            break;
        }
        case 0x3B: // DCX SP
            state->sp = state->sp - 1;
            state->b = (state->sp & 0xff00) >> 8;
            state->l = state->sp & 0xff;
            break;
        case 0x3C: // INR A
            state->a = state->a + 1;
            state->cc.z = ((state->a) == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.ac = ((state->a & 0xf) == 0xf);
            break;
        case 0x3D: // DCR A
            state->a = state->a - 1;
            state->cc.z = ((state->a) == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.ac = ((state->a & 0xf) == 0x0);
            break;
        case 0x3E: // MVI A, byte
            state->a = opcode[1];
            state->pc++; // for the data byte
            break;
        case 0x3F: // CMC
            state->cc.cy = !state->cc.cy;
            break;
        case 0x40: // MOV B, B
            state->b = state->b;
            break;
        case 0x41: // MOV B, C
            state->b = state->c;
            break;
        case 0x42: // MOV B, D
            state->b = state->d;
            break;
        case 0x43: // MOV B, E
            state->b = state->e;
            break;
        case 0x44: // MOV B, H
            state->b = state->h;
            break;
        case 0x45: // MOV B, L
            state->b = state->l;
            break;
        case 0x46: // MOV B, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->b = state->memory[offset];
            break;
        }
        case 0x47: // MOV B, A
            state->b = state->a;
            break;
        case 0x48: // MOV C, B
            state->c = state->b;
            break;
        case 0x49: // MOV C, C
            state->c = state->c;
            break;
        case 0x4A: // MOV C, D
            state->c = state->d;
            break;
        case 0x4B: // MOV C, E
            state->c = state->e;
            break;
        case 0x4C: // MOV C, H
            state->c = state->h;
            break;
        case 0x4D: // MOV C, L
            state->c = state->l;
            break;
        case 0x4E: // MOV C, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->c = state->memory[offset];
            break;
        }
        case 0x4F: // MOV C, A
            state->c = state->a;
            break;
        case 0x50: // MOV D, B
            state->d = state->b;
            break;
        case 0x51: // MOV D, C
            state->d = state->c;
            break;
        case 0x52: // MOV D, D
            state->d = state->d;
            break;
        case 0x53: // MOV D, E
            state->d = state->e;
            break;
        case 0x54: // MOV D, H
            state->d = state->h;
            break;
        case 0x55: // MOV D, L
            state->d = state->l;
            break;
        case 0x56: // MOV D, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->d = state->memory[offset];
            break;
        }
        case 0x57: // MOV D, A
            state->d = state->a;
            break;
        case 0x58: // MOV E, B
            state->e = state->b;
            break;
        case 0x59: // MOV E, C
            state->e = state->c;
            break;
        case 0x5A: // MOV E, D
            state->e = state->d;
            break;
        case 0x5B: // MOV E, E
            state->e = state->e;
            break;
        case 0x5C: // MOV E, H
            state->e = state->h;
            break;
        case 0x5D: // MOV E, L
            state->e = state->l;
            break;
        case 0x5E: // MOV E, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->e = state->memory[offset];
            break;
        }
        case 0x5F: // MOV E, A
            state->e = state->a;
            break;
        case 0x60: // MOV H, B
            state->h = state->b;
            break;
        case 0x61: // MOV H, C
            state->h = state->c;
            break;
        case 0x62: // MOV H, D
            state->h = state->d;
            break;
        case 0x63: // MOV H, E
            state->h = state->e;
            break;
        case 0x64: // MOV H, H
            state->h = state->h;
            break;
        case 0x65: // MOV H, L
            state->h = state->l;
            break;
        case 0x66: // MOV H, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->h = state->memory[offset];
            break;
        }
        case 0x67: // MOV H, A
            state->h = state->a;
            break;
        case 0x68: // MOV L, B
            state->l = state->b;
            break;
        case 0x69: // MOV L, C
            state->l = state->c;
            break;
        case 0x6A: // MOV L, D
            state->l = state->d;
            break;
        case 0x6B: // MOV L, E
            state->l = state->e;
            break;
        case 0x6C: // MOV L, H
            state->l = state->h;
            break;
        case 0x6D: // MOV L, L
            state->l = state->l;
            break;
        case 0x6E: // MOV L, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->l = state->memory[offset];
            break;
        }
        case 0x6F: // MOV L, A
            state->l = state->a;
            break;
        case 0x70: // MOV M, B
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->b;
            break;
        }
        case 0x71: // MOV M, C
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->c;
            break;
        }
        case 0x72: // MOV M, D
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->d;
            break;
        }
        case 0x73: // MOV M, E
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->e;
            break;
        }
        case 0x74: // MOV M, H
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->h;
            break;
        }
        case 0x75: // MOV M, L
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->l;
            break;
        }
        case 0x76: // HLT
            exit(0);
        case 0x77: // MOV B, A
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->memory[offset] = state->a;
            break;
        }
        case 0x78: // MOV A, B
            state->a = state->b;
            break;
        case 0x79: // MOV A, C
            state->a = state->c;
            break;
        case 0x7A: // MOV A, D
            state->a = state->d;
            break;
        case 0x7B: // MOV A, E
            state->a = state->e;
            break;
        case 0x7C: // MOV A, H
            state->a = state->h;
            break;
        case 0x7D: // MOV A, L
            state->a = state->l;
            break;
        case 0x7E: // MOV A, M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->a = state->memory[offset];
            break;
        }
        case 0x7F: // MOV A, A
            state->a = state->a;
            break;
        case 0x80: // ADD B
        {
            // do math with higher precision so we can capture the
            // carry out
            uint16_t answer = (uint16_t) state->a + (uint16_t) state ->b;

            // Zero flag: if the result is zero,
            // set the flag to zero
            // else clear the flag
            if ((answer & 0xff) == 0)
                state->cc.z = 1;
            else
                state->cc.z = 0;

            // Sign flag: if bit 7 is set,
            // set the sign flag
            // else clear the sign flag
            if (answer & 0x80)
                state->cc.s = 1;
            else
                state->cc.s = 0;

            // Carry flag
            if (answer > 0xff)
                state->cc.cy = 1;
            else
                state->cc.cy = 0;

            // Parity is handled by a subroutine
            state->cc.p = parity(answer & 0xff, 8);

            // Auxiliary Carry
            if ((state->a & 0xf) + (state->b & 0xf) > 0xf)
                state->cc.ac = 1;
            else
                state->cc.ac = 0;

            state->a = answer & 0xff;
            break;
        }
        // The code for ADD can be condensed like this
        case 0x81: // ADD C
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->c;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->c & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x82: // ADD D
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->d;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->d & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x83: // ADD E
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->e;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->e & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x84: // ADD H
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->h;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->h & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x85: // ADD L
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->l;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->l & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x86: // ADD M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a + state->memory[offset];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->memory[offset] & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x87: // ADD A
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->a;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->a & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x88: // ADC B
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->b + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x89: // ADC C
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->c + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->c & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x8A: // ADC D
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->d + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->d & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x8B: // ADC E
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->e + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->e & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x8C: // ADC H
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->h + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->h & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x8D: // ADC L
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->l + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->l & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x8E: // ADC M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a + state->memory[offset] + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->memory[offset] & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x8F: // ADC A
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) state->a + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (state->a & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x90: // SUB B
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->b;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x91: // SUB C
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->c;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x92: // SUB D
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->d;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->d) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x93: // SUB E
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->e;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->e) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x94: // SUB H
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->h;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->h) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x95: // SUB L
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->l;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->l) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x96: // SUB M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a - state->memory[offset];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->memory[offset]) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x97: // SUB A
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) state->a;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->a) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x98: // SBB B
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->b + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->b + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x99: // SBB C
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->c + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x9A: // SBB D
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->d + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->d + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x9B: // SBB E
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->e + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->e + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x9C: // SBB H
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->h + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->h + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x9D: // SBB L
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->l + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->l + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x9E: // SBB M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            uint16_t answer = (uint16_t) state->a - (state->memory[offset] + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->memory[offset] + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0x9F: // SBB A
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) state->a + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->a + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0xA0: // ANA B
            state->a = state->a & state->b;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA1: // ANA C
            state->a = state->a & state->c;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA2: // ANA D
            state->a = state->a & state->d;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA3: // ANA E
            state->a = state->a & state->e;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA4: // ANA H
            state->a = state->a & state->h;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA5: // ANA L
            state->a = state->a & state->l;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA6: // ANA M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->a = state->a & state->memory[offset];
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        }
        case 0xA7: // ANA A
            state->a = state->a & state->a;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA8: // XRA B
            state->a = state->a ^ state->b;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xA9: // XRA C
            state->a = state->a ^ state->c;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xAA: // XRA D
            state->a = state->a ^ state->d;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xAB: // XRA E
            state->a = state->a ^ state->e;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xAC: // XRA H
            state->a = state->a ^ state->h;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xAD: // XRA L
            state->a = state->a ^ state->l;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xAE: // XRA M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->a = state->a ^ state->memory[offset];
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        }
        case 0xAF: // XRA A
            state->a = state->a ^ state->a;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB0: // ORA B
            state->a = state->a | state->b;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB1: // ORA C
            state->a = state->a | state->c;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB2: // ORA D
            state->a = state->a | state->d;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB3: // ORA E
            state->a = state->a | state->e;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB4: // ORA H
            state->a = state->a | state->h;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB5: // ORA L
            state->a = state->a | state->l;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB6: // ORA M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            state->a = state->a | state->memory[offset];
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        }
        case 0xB7: // ORA A
            state->a = state->a | state->a;
            state->cc.z = (state->a == 0);
            state->cc.s = ((state->a & 0x80) == 0x80);
            state->cc.p = parity(state->a, 8);
            state->cc.cy = 0; // clears CY
            state->cc.ac = 0; // clears AC
            break;
        case 0xB8: // CMP B
        {
            // Like SUB B but without storing
            uint16_t res = (uint16_t) state->a - (uint16_t) state->b;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xB9: // CMP C
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) state->c;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xBA: // CMP D
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) state->d;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xBB: // CMP E
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) state->e;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xBC: // CMP H
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) state->h;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xBD: // CMP L
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) state->l;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xBE: // CMP M
        {
            uint16_t offset = (state->h << 8) | (state->l);
            uint16_t res = (uint16_t) state->a - (uint16_t) state->memory[offset];
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        case 0xBF: // CMP A
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) state->l;
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            break;
        }
        /*....*/
        case 0xC0: // RNZ
            if (state->cc.z == 0)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xC1: // POP B
            state->c = state->memory[state->sp];
            state->b = state->memory[state->sp+1];
            state->sp += 2;
            break;
        case 0xC2: // JNZ address
            if (state->cc.z == 0)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xC3: // JMP address
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        case 0xC4: // CNZ address
        {
            if (state->cc.z == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xC5: // PUSH B
            state->memory[state->sp-1] = state->b;
            state->memory[state->sp-2] = state->c;
            state->sp = state->sp - 2;
            break;
        case 0xC6: // ADI byte
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (opcode[1] & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0xC7: // RST 0
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x0;
            break;
        }
        case 0xC8: // RZ
            if (state->cc.z == 1)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xC9: // RET
            state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            state->sp += 2;
            break;
        case 0xCA: // JZ address
            if (state->cc.z == 1)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xCC: // CZ address
        {
            if (state->cc.z == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xCD: // CALL address
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        }
        case 0xCE: // ACI byte
        {
            uint16_t answer = (uint16_t) state->a + (uint16_t) opcode[1] + state->cc.cy;
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (opcode[1] & 0xf) + state->cc.cy) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0xCF: // RST 1
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x8;
            break;
        }
        case 0xD0: // RNC
            if (state->cc.cy == 0)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xD1: // POP D
            state->e = state->memory[state->sp];
            state->d = state->memory[state->sp+1];
            state->sp += 2;
            break;
        case 0xD2: // JNC address
            if (state->cc.cy == 0)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xD3: // OUT byte
        {
            uint8_t port = opcode[1];
            machineOUT(state, port);
            state->pc++;
            break;
        }
        case 0xD4: // CNC address
        {
            if (state->cc.cy == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xD5: // PUSH D
            state->memory[state->sp-1] = state->d;
            state->memory[state->sp-2] = state->e;
            state->sp = state->sp - 2;
            break;
        case 0xD6: // SUI byte
        {
            uint16_t answer = (uint16_t) state->a - (uint16_t) opcode[1];
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(opcode[1]) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0xD7: // RST 2
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x10;
            break;
        }
        case 0xD8: // RC
            if (state->cc.cy == 1)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xDA: // JC address
            if (state->cc.cy == 1)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xDB: // IN byte
        {
            uint8_t port = opcode[1];
            state->a = machineIN(state, port);
            state->pc++;
            break;
        }
        case 0xDC: // CC address
        {
            if (state->cc.cy == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xDE: // SCI byte
        {
            uint16_t answer = (uint16_t) state->a - ((uint16_t) opcode[1] + state->cc.cy);
            state->cc.z = ((answer & 0xff) == 0);
            state->cc.s = ((answer & 0x80) == 0x80);
            state->cc.cy = (answer > 0xff);
            state->cc.p = parity(answer & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(opcode[1] + state->cc.cy) & 0xf)) > 0xf);
            state->a = answer & 0xff;
            break;
        }
        case 0xDF: // RST 3
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x18;
            break;
        }
        case 0xE0: // RPO
            if (state->cc.p == 0)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xE1: // POP H
            state->l = state->memory[state->sp];
            state->h = state->memory[state->sp+1];
            state->sp += 2;
            break;
        case 0xE2: // JPO address
            if (state->cc.p == 0)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xE3: // XTHL
        {
            uint8_t swap = state->l;
            state->l = state->memory[state->sp];
            state->memory[state->sp] = swap;

            swap = state->h;
            state->h = state->memory[state->sp+1];
            state->memory[state->sp+1] = swap;
            break;
        }
        case 0xE4: // CPO address
        {
            if (state->cc.p == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xE5: // PUSH H
            state->memory[state->sp-1] = state->h;
            state->memory[state->sp-2] = state->l;
            state->sp = state->sp - 2;
            break;
        case 0xE6: // ANI byte
        {
            uint8_t x = state-> a & opcode[1];
            state->cc.z = (x == 0);
            state->cc.s = ((x & 0x80) == 0x80);
            state->cc.p = parity(x, 8);
            state->cc.cy = 0; // Data book says op clears carry flags
            state->cc.ac = 0;
            state->a = x;
            state->pc++; // for the data byte
            break;
        }
        case 0xE7: // RST 4
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x20;
            break;
        }
        case 0xE8: // RPE
            if (state->cc.p == 1)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xE9: // PCHL
            state->pc = (state->h << 8) | (state->l);
            break;
        case 0xEA: // JPE address
            if (state->cc.p == 1)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xEB: // XCHG
        {
            uint8_t swap = state->l;
            state->l = state->e;
            state->e = swap;

            swap = state->h;
            state->h = state->d;
            state->d = swap;
            break;
        }
        case 0xEC: // CPE address
        {
            if (state->cc.p == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xEE: // XRI byte
        {
            uint8_t x = state-> a ^ opcode[1];
            state->cc.z = (x == 0);
            state->cc.s = ((x & 0x80) == 0x80);
            state->cc.p = parity(x, 8);
            state->cc.cy = 0; // Data book says op clears carry flags
            state->cc.ac = 0;
            state->a = x;
            state->pc++; // for the data byte
            break;
        }
        case 0xEF: // RST 5
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x28;
            break;
        }
        case 0xF0: // RP
            if (state->cc.s == 0)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xF1: // POP PSW
        {
            state->a = state->memory[state->sp+1];
            uint8_t psw = state->memory[state->sp];
            state->cc.z = ((psw & 0x01) == 0x01);
            state->cc.s = ((psw & 0x02) == 0x02);
            state->cc.p = ((psw & 0x04) == 0x04);
            state->cc.cy = ((psw & 0x08) == 0x08);
            state->cc.ac = ((psw & 0x10) == 0x10);
            state->sp += 2;
            break;
        }
        case 0xF2: // JP address
            if (state->cc.s == 0)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xF3: // DI
            state->int_enable = 0;
            break;
        case 0xF4: // CP address
        {
            if (state->cc.s == 0)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xF5: // PUSH PSW
        {
            state->memory[state->sp-1] = state->a;
            uint8_t psw = (state->cc.z  |
                           state->cc.s  << 1 |
                           state->cc.p  << 2 |
                           state->cc.cy << 3 |
                           state->cc.ac << 4 );
            state->memory[state->sp-2] = psw;
            state->sp = state->sp - 2;
            break;
        }
        case 0xF6: // ORI byte
        {
            uint8_t x = state-> a | opcode[1];
            state->cc.z = (x == 0);
            state->cc.s = ((x & 0x80) == 0x80);
            state->cc.p = parity(x, 8);
            state->cc.cy = 0; // Data book says op clears carry flags
            state->cc.ac = 0;
            state->a = x;
            state->pc++; // for the data byte
            break;
        }
        case 0xF7: // RST 6
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x30;
            break;
        }
        case 0xF8: // RM
            if (state->cc.s == 1)
            {
                state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
                state->sp += 2;
            }
            break;
        case 0xF9: // SPHL
            state->sp = (state->h << 8) | (state->l);
            break;
        case 0xFA: // JM address
            if (state->cc.s == 1)
                state->pc = (opcode[2] << 8) | opcode[1];
            else
                // branch not taken
                state->pc += 2;
            break;
        case 0xFB: // EI
            state->int_enable = 1;
            break;
        case 0xFC: // CM address
        {
            if (state->cc.s == 1)
            {
                uint16_t ret = state->pc + 2;
                state->memory[state->sp-1] = (ret >> 8) & 0xff;
                state->memory[state->sp-2] = (ret & 0xff);
                state->sp = state->sp - 2;
                state->pc = (opcode[2] << 8) | opcode[1];
            }
            else
                state->pc += 2;
            break;
        }
        case 0xFE: // CPI byte
        {
            uint16_t res = (uint16_t) state->a - (uint16_t) opcode[1];
            state->cc.z = ((res & 0xff) == 0);
            state->cc.s = ((res & 0x80) == 0x80);
            state->cc.cy = (res > 0xff);
            state->cc.p = parity(res & 0xff, 8);
            state->cc.ac = (((state->a & 0xf) + (-(state->c) & 0xf)) > 0xf);
            state->pc++; // for the data byte
            break;
        }
        case 0xFF: // RST 7
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x38;
            break;
        }
        default:
            unimplementedInstruction(state);
            break;
    }

    return 1;
}

void push(State8080 *state, uint8_t msbReg, uint8_t lsbReg) {
	state->memory[state->sp - 1] = msbReg;
	state->memory[state->sp - 2] = lsbReg;

	state->sp -= 2;
}

void generateInterrupt(State8080 *state, int interrupt_num) {
	//Push PC
	push(state, ((state->pc & 0xff00) >> 8), (state->pc & 0xff));

	//Set PC to low memory vector
	//RST interrupt_num
	state->pc = 8 * interrupt_num;
	state->int_enable = 0;
}

void readFileIntoMemoryAt(State8080* state, char* filename, uint32_t offset) {
    FILE *f= fopen(filename, "rb");
    if (f==NULL) {
        printf("error: Couldn't open %s\n", filename);
        exit(1);
    }
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    uint8_t *buffer = &state->memory[offset];
    fread(buffer, fsize, 1, f);
    fclose(f);
}

State8080* init8080(void) {
    State8080* state = calloc(1, sizeof(State8080));
    state->memory = malloc(0x10000);  //16K
    return state;
}

//Returns time in microseconds
double timeusec() {
    //get time
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    //convert from seconds to microseconds
    return ((double)currentTime.tv_sec * 1E6) + ((double)currentTime.tv_usec);
}

static void
activate (GtkApplication* app, gpointer user_data) {
  GtkWidget *window;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_widget_show_all (window);
}

int main (int argc, char**argv) {

    State8080* state = init8080();

    readFileIntoMemoryAt(state, "games/space-invaders/invaders.h", 0);
    readFileIntoMemoryAt(state, "games/space-invaders/invaders.g", 0x800);
    readFileIntoMemoryAt(state, "games/space-invaders/invaders.f", 0x1000);
    readFileIntoMemoryAt(state, "games/space-invaders/invaders.e", 0x1800);

	GtkApplication *app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    int done = 1;
	double now;
	int whichInt = 1;
	double lastInterrupt = 0.0;
	while (done == 1)
    {
        done = emulate8080(state);

		now = timeusec();
        if ( now - lastInterrupt > 16667) // 1/60 seconds has elapsed
        {
            // only do an interrupt if they are enabled
            if (state->int_enable) {
				if (whichInt == 1) {
					whichInt = 2;
				}
				if (whichInt == 2) {
					whichInt = 1;
				}
				generateInterrupt(state, whichInt); // Interrupt 2
                // Save the time we did this
                lastInterrupt = now;
            }
        }
    }

    return 0;
}