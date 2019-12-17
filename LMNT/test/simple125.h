#pragma once

static const char filedata_simple125[] = {
    'L', 'M', 'N', 'T',
    0x00, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x00, 0x00, // strings length
    0x15, 0x00, 0x00, 0x00, // defs length
    0xF2, 0x03, 0x00, 0x00, // code length
    0x00, 0x00, 0x00, 0x00, // constants_length
    0x07, 0x00, 'P', 'o', 't', 'a', 't', 'o', '\0', // strings[0]
    0x15, 0x00, // defs[0].length
    0x00, 0x00, // defs[0].name
    0x00, 0x00, // defs[0].flags
    0x00, 0x00, 0x00, 0x00, // defs[0].code
    0x03, 0x00, // stack_size_unaligned
    0x03, 0x00, // stack_size_aligned
    0x00, 0x00, // defs[0].bases_size
    0x02, 0x00, // defs[0].args_size
    0x01, 0x00, // defs[0].rvals_size
    0x00,       // defs[0].bases_count
    // code
    0x7D, 0x00, 0x00, 0x00, // ops_count
    LMNT_OP(LMNT_OP_ADDSS, 0x00, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    LMNT_OP(LMNT_OP_ADDSS, 0x02, 0x01, 0x02),
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
