#pragma once

#include "lmnt/opcodes.h"

static const char filedata_circle[] = {
    'L', 'M', 'N', 'T',
    0x00, 0x00, 0x00, 0x00,
    0x0C, 0x00, 0x00, 0x00, // strings length
    0x10, 0x00, 0x00, 0x00, // defs length
    0x6C, 0x00, 0x00, 0x00, // code length
    0x04, 0x00, 0x00, 0x00, // data length
    0x08, 0x00, 0x00, 0x00, // constants_length
    0x0A, 0x00, 'C', 'i', 'r', 'c', 'l', 'e', '\0', 0x00, 0x00, 0x00, // strings[0]
    0x00, 0x00, // defs[0].name
    0x00, 0x00, // defs[0].flags
    0x00, 0x00, 0x00, 0x00, // defs[0].code
    0x0B, 0x00, // defs[0].stack_count
    0x04, 0x00, // defs[0].args_count
    0x07, 0x00, // defs[0].rvals_count
    0x00, 0x00, // defs[0].default_args_index
    // code
    0x0D, 0x00, 0x00, 0x00, // ops_count
    // stack: [tau, 1.0 | time_i, time_f, radius, interval | pos_x, pos_y, pos_z, dir_x, dir_y, dir_z, intens]
    //temp7 = ((time_i % interval) + time_f) % interval
    LMNT_OP_BYTES(LMNT_OP_REMSS, 0x02, 0x05, 0x07),
    LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x07, 0x03, 0x07),
    LMNT_OP_BYTES(LMNT_OP_REMSS, 0x07, 0x05, 0x07),
    //temp7 = div(temp7, interval)
    LMNT_OP_BYTES(LMNT_OP_DIVSS, 0x07, 0x05, 0x07),
    //temp7 = mul(temp7, tau)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x07, 0x00, 0x07),
    //pos_x = cos(temp7), pos_y = sin(temp7)
    LMNT_OP_BYTES(LMNT_OP_SINCOS, 0x07, 0x06, 0x07),
    //pos_x = mul(pos_x, radius)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x06, 0x04, 0x06),
    //pos_y = mul(pos_y, radius)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x07, 0x04, 0x07),
    //stack[8] = 0.15
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x999A, 0x3E19, 0x08),
    //stack[9] = 0.0
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x09),
    //stack[A] = 0.0
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x0A),
    //stack[B] = 1.0
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x0B),
    //stack[C] = 1.0
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x0C),
    // data
    0x00, 0x00, 0x00, 0x00,
    // constants
    0xDB, 0x0F, 0xC9, 0x40, // tau
    0x00, 0x00, 0x80, 0x3F, // 1.0
};
