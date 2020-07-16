#pragma once

static const char filedata_circle_double[] = {
    'L', 'M', 'N', 'T',
    0x00, 0x00, 0x00, 0x00,
    0x12, 0x00, 0x00, 0x00, // strings length
    0x2A, 0x00, 0x00, 0x00, // defs length
    0x5C, 0x00, 0x00, 0x00, // code length
    0x08, 0x00, 0x00, 0x00, // constants_length
    0x07, 0x00, 'd', 'o', 'u', 'b', 'l', 'e', '\0', // strings[0]
    0x07, 0x00, 'C', 'i', 'r', 'c', 'l', 'e', '\0', // strings[1]
    0x15, 0x00, // defs[0].length
    0x00, 0x00, // defs[0].name
    0x02, 0x00, // defs[0].flags
    0xFF, 0xFF, 0xFF, 0xFF, // defs[0].code
    0x02, 0x00, // defs[0].stack_count_unaligned
    0x02, 0x00, // defs[0].stack_count_aligned
    0x00, 0x00, // defs[0].base_args_count
    0x01, 0x00, // defs[0].args_count
    0x01, 0x00, // defs[0].rvals_count
    0x00,       // defs[0].bases_count
    0x15, 0x00, // defs[0].length
    0x09, 0x00, // defs[0].name
    0x00, 0x00, // defs[0].flags
    0x00, 0x00, 0x00, 0x00, // defs[0].code
    0x08, 0x00, // defs[0].stack_count_unaligned
    0x08, 0x00, // defs[0].stack_count_aligned
    0x00, 0x00, // defs[0].base_args_count
    0x04, 0x00, // defs[0].args_count
    0x04, 0x00, // defs[0].rvals_count
    0x00,       // defs[0].bases_count
    // code
    0x0B, 0x00, 0x00, 0x00, // ops_count
    // stack: [tau, 1.0 | t, i, radius, interval | pos_x, pos_y, pos_z, intens]
    //temp7 = t / interval
    LMNT_OP_BYTES(LMNT_OP_DIVSS, 0x02, 0x05, 0x07),
    //temp7 = mod(temp7, 1)
    LMNT_OP_BYTES(LMNT_OP_MODSS, 0x07, 0x01, 0x07),
    //temp7 = mul(temp7, tau)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x07, 0x00, 0x07),
    //pos_x = cos(temp7)
    LMNT_OP_BYTES(LMNT_OP_COS, 0x07, 0x00, 0x06),
    //pos_x = mul(pos_x, radius)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x06, 0x04, 0x06),
    //pos_y = sin(temp7)
    LMNT_OP_BYTES(LMNT_OP_SIN, 0x07, 0x00, 0x07),
    //pos_y = mul(pos_y, radius)
    LMNT_OP_BYTES(LMNT_OP_MULSS, 0x07, 0x04, 0x07),
    //stack[8] = 0.15
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x999A, 0x3E19, 0x08),
    //stack[9] = double_a_thing(stack[8])
    LMNT_OP_BYTES(LMNT_OP_EXTCALL, 0x00, 0x00, 0x08),
    //stack[8] = stack[9]
    LMNT_OP_BYTES(LMNT_OP_ASSIGNSS, 0x09, 0x00, 0x08),
    //stack[9] = 1.0
    LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x09),
    // pad to multiple of 8
    // constants
    0xDB, 0x0F, 0xC9, 0x40, // tau
    0x00, 0x00, 0x80, 0x3F, // 1.0
};
