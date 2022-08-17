#pragma once

#include "lmnt/opcodes.h"

static const char filedata_circle_ht[] = {
    'L', 'M', 'N', 'T',
    0x00, 0x00, 0x00, 0x00,
    0x0C, 0x00, 0x00, 0x00, // strings length
    0x10, 0x00, 0x00, 0x00, // defs length
    0xC4, 0x00, 0x00, 0x00, // code length
    0x04, 0x00, 0x00, 0x00, // data length
    0x08, 0x00, 0x00, 0x00, // constants_length
    0x0A, 0x00, 'C', 'i', 'r', 'c', 'l', 'e', 'H', 'T', '\0', 0x00, // strings[0]
    0x00, 0x00, // defs[0].name
    0x00, 0x00, // defs[0].flags
    0x00, 0x00, 0x00, 0x00, // defs[0].code
    0x1B, 0x00, // defs[0].stack_count
    0x10, 0x00, // defs[0].args_count
    0x07, 0x00, // defs[0].rvals_count
    0x00, 0x00, // defs[0].default_args_index
    // code
    0x18, 0x00, 0x00, 0x00, // ops_count
    // stack: [
    // 0x00 | tau, 1.0
    // 0x02 | time_i, time_f, radius, interval, t11, t12, t13, t14, t21, t22, t23, t24, t31, t32, t33, t34
    // 0x12 | pos_x, pos_y, pos_z, intens
    // 0x16 | temp22, temp23, temp24, temp25, temp26
    // ]
    //temp22 = ((time_i % interval) + time_f) % interval
    LMNT_OP_BYTES(LMNT_OP_REMSS, 0x02, 0x05, 0x16),
    LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x16, 0x03, 0x16),
    LMNT_OP_BYTES(LMNT_OP_REMSS, 0x16, 0x05, 0x16),
    //temp22 = div(temp22, interval)
    LMNT_OP_BYTES(LMNT_OP_DIVSS, 0x16, 0x05, 0x16),
    //temp22 = mul(temp22, tau)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x16, 0x00, 0x16),
    //pos_x = cos(temp22), pos_y = sin(temp22)
    LMNT_OP_BYTES(LMNT_OP_SINCOS, 0x16, 0x12, 0x13),
    //pos_x = mul(pos_x, radius)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x12, 0x04, 0x12),
    //pos_y = mul(pos_y, radius)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x13, 0x04, 0x13),
    //pos_z = 0.00
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x14),
    //intensity = 0.0 (to not break summing later)
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x15),
    //temp22 = t14
    LMNT_OP_BYTES(LMNT_OP_ASSIGNSS, 0x09, 0x00, 0x16),
    //temp23 = t24
    LMNT_OP_BYTES(LMNT_OP_ASSIGNSS, 0x0D, 0x00, 0x17),
    //temp24 = t34
    LMNT_OP_BYTES(LMNT_OP_ASSIGNSS, 0x11, 0x00, 0x18),
    //t1 = t1 * pos
    LMNT_OP_BYTES(LMNT_OP_MULVV, 0x06, 0x12, 0x06),
    //t2 = t2 * pos
    LMNT_OP_BYTES(LMNT_OP_MULVV, 0x0A, 0x12, 0x0A),
    //t3 = t3 * pos
    LMNT_OP_BYTES(LMNT_OP_MULVV, 0x0E, 0x12, 0x0E),
    //pos_x = sum(t1)
    LMNT_OP_BYTES(LMNT_OP_SUMV, 0x06, 0x00, 0x12),
    //pos_y = sum(t2)
    LMNT_OP_BYTES(LMNT_OP_SUMV, 0x0A, 0x00, 0x13),
    //pos_z = sum(t3)
    LMNT_OP_BYTES(LMNT_OP_SUMV, 0x0E, 0x00, 0x14),
    // pos = pos + [t14, t24, t34, ???]
    LMNT_OP_BYTES(LMNT_OP_ADDVV, 0x12, 0x16, 0x12),
    // dir = 0,0,1
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x15),
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x16),
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x17),
    //intensity = 1.0
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x18),
    // data
    0x00, 0x00, 0x00, 0x00,
    // constants
    0xDB, 0x0F, 0xC9, 0x40, // tau
    0x00, 0x00, 0x80, 0x3F, // 1.0
};
