#ifndef LMNT_OPCODES_H
#define LMNT_OPCODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint16_t lmnt_opcode;

enum
{
    // no-op: null, null, null
    LMNT_OP_NOOP         = 0x01,
    LMNT_OP_RETURN       = 0x00,
    // assign variants: stack, null, stack
    LMNT_OP_ASSIGNSS     = 0x02,
    LMNT_OP_ASSIGNVV     = 0x03,
    LMNT_OP_ASSIGNSV     = 0x05,
    // assign variants: immlo, immhi, stack
    LMNT_OP_ASSIGNIBS    = 0x08,
    LMNT_OP_ASSIGNIBV    = 0x09,
    // data load: immediate, immediate, stack
//    LMNT_OP_DLOADIIS,
//    LMNT_OP_DLOADIIV,
    // data load: immediate, data ref, stack
//    LMNT_OP_DLOADIRS,
//    LMNT_OP_DLOADIRV,
    // data section get count: immediate, null, stack
//    LMNT_OP_DSECLEN,
    // add: stack, stack, stack
    LMNT_OP_ADDSS        = 0x0C,
//    LMNT_OP_ADDVV,
    // sub: stack, stack, stack
    LMNT_OP_SUBSS        = 0x0D,
//    LMNT_OP_SUBVV,
    // mul: stack, stack, stack
    LMNT_OP_MULSS        = 0x0E,
//    LMNT_OP_MULVV,
    // div: stack, stack, stack
    LMNT_OP_DIVSS        = 0x0F,
//    LMNT_OP_DIVVV,
    // rem: stack, stack, stack
    LMNT_OP_REMSS        = 0x13,
//    LMNT_OP_REMVV,
    // trig: stack, null, stack
    LMNT_OP_SINR         = 0x1D,
    LMNT_OP_COSR         = 0x1E,
    LMNT_OP_TANR         = 0x1F,
    LMNT_OP_ASINR        = 0x24,
    LMNT_OP_ACOSR        = 0x2C,
    LMNT_OP_ATANR        = 0x34,
    // trig: stack, stack, stack
    LMNT_OP_ATAN2R       = 0x36,
    LMNT_OP_SINCOSR      = 0x37,
    // pow: stack, stack, stack
    LMNT_OP_POWSS        = 0x38,
//    LMNT_OP_POWVV,
//    LMNT_OP_POWVS,
    // sqrt: stack, null, stack
    LMNT_OP_SQRTS        = 0x3E,
//    LMNT_OP_SQRTV,
    // logn: stack, null, stack
//    LMNT_OP_LN,
    LMNT_OP_LOG2         = 0x40,
//    LMNT_OP_LOG10,
    // abs: stack, null, stack
    LMNT_OP_ABSS         = 0x42,
//    LMNT_OP_ABSV,
    // sum: stack, null, stack
    LMNT_OP_SUMV         = 0x6B,
    // min/max: stack, stack, stack
    LMNT_OP_MINSS        = 0x43,
//    LMNT_OP_MINVV,
    LMNT_OP_MAXSS        = 0x46,
//    LMNT_OP_MAXVV,
    // min/max: stack, null, stack
//    LMNT_OP_MINVS,
//    LMNT_OP_MAXVS,
    // rounding: stack, null, stack
    LMNT_OP_FLOORS       = 0x49,
//    LMNT_OP_FLOORV,
    LMNT_OP_ROUNDS       = 0x4A,
//    LMNT_OP_ROUNDV,
    LMNT_OP_CEILS        = 0x4B,
//    LMNT_OP_CEILV,
//    LMNT_OP_TRUNCS,
//    LMNT_OP_TRUNCV,
    // indexing: stackref, immediate, stack
    LMNT_OP_INDEXRIS     = 0x4C,
    // indexing: stackref, immediate, stackref
    LMNT_OP_INDEXRIR     = 0x4E,
    // branch: null, codelo, codehi
    LMNT_OP_BRANCH       = 0x50,
    // branch: stack, codelo, codehi
    LMNT_OP_BRANCHZ      = 0x51,
    LMNT_OP_BRANCHNZ     = 0x53,
    LMNT_OP_BRANCHPOS    = 0x55,
    LMNT_OP_BRANCHNEG    = 0x57,
    LMNT_OP_BRANCHUN     = 0x59,
    // compare: stack, stack, null
    LMNT_OP_CMP          = 0x5B,
    // compare: stack, null, null
    LMNT_OP_CMPZ         = 0x5C,
    // branch: null, codelo, codehi
    LMNT_OP_BRANCHCEQ    = 0x5D,
    LMNT_OP_BRANCHCNE    = 0x5E,
    LMNT_OP_BRANCHCLT    = 0x5F,
    LMNT_OP_BRANCHCLE    = 0x60,
    LMNT_OP_BRANCHCGT    = 0x61,
    LMNT_OP_BRANCHCGE    = 0x62,
    LMNT_OP_BRANCHCUN    = 0x63,
    // conditional assign: stack, stack, stack
    LMNT_OP_ASSIGNCEQ    = 0x64,
    LMNT_OP_ASSIGNCNE    = 0x65,
    LMNT_OP_ASSIGNCLT    = 0x66,
    LMNT_OP_ASSIGNCLE    = 0x67,
    LMNT_OP_ASSIGNCGT    = 0x68,
    LMNT_OP_ASSIGNCGE    = 0x69,
    LMNT_OP_ASSIGNCUN    = 0x6A,
    // extern call: deflo, defhi, stack
//    LMNT_OP_EXTCALL,
    // placeholder end operation
    LMNT_OP_END          = 0x6C, // larger than any valid entry
};

#define LMNT_IS_BRANCH_OP(op) (((op) >= LMNT_OP_BRANCH && (op) <= LMNT_OP_BRANCHUN) || ((op) >= LMNT_OP_BRANCHCEQ && ((op) <= LMNT_OP_BRANCHCUN)))


typedef enum
{
    LMNT_OPERAND_UNUSED,
    LMNT_OPERAND_STACK1,   // S
    LMNT_OPERAND_STACK4,   // V
    LMNT_OPERAND_STACKN,
    LMNT_OPERAND_IMM,      // I
    LMNT_OPERAND_STACKREF, // R
    LMNT_OPERAND_DEFPTR,
    LMNT_OPERAND_CODEPTR,
} lmnt_operand_type;

typedef struct
{
    const char* name;
    lmnt_operand_type operand1;
    lmnt_operand_type operand2;
    lmnt_operand_type operand3;
} lmnt_op_info;

const lmnt_op_info* lmnt_get_opcode_info(lmnt_opcode op);

#define LMNT_OP16(a) (char)(a & 0xFF), (char)((a >> 8) & 0xFF)
#define LMNT_OP_BYTES(op, arg1, arg2, arg3) LMNT_OP16(op), LMNT_OP16(arg1), LMNT_OP16(arg2), LMNT_OP16(arg3)


#ifdef __cplusplus
}
#endif

#endif