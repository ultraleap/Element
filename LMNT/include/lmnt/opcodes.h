#ifndef LMNT_OPCODES_H
#define LMNT_OPCODES_H

#include <stdint.h>

typedef uint16_t lmnt_opcode;

enum
{
    // no-op: null, null, null
    LMNT_OP_NOOP = 0,
    // assign variants: stack, null, stack
    LMNT_OP_ASSIGNSS,
    LMNT_OP_ASSIGNVV,
    LMNT_OP_ASSIGNSV,
    // assign variants: immlo, immhi, stack
    LMNT_OP_ASSIGNIIS,
    LMNT_OP_ASSIGNIBS,
    LMNT_OP_ASSIGNIIV,
    LMNT_OP_ASSIGNIBV,
    // add: stack, stack, stack
    LMNT_OP_ADDSS,
    LMNT_OP_ADDVV,
    // sub: stack, stack, stack
    LMNT_OP_SUBSS,
    LMNT_OP_SUBVV,
    // mul: stack, stack, stack
    LMNT_OP_MULSS,
    LMNT_OP_MULVV,
    // div: stack, stack, stack
    LMNT_OP_DIVSS,
    LMNT_OP_DIVVV,
    // mod: stack, stack, stack
    LMNT_OP_MODSS,
    LMNT_OP_MODVV,
    // trig: stack, null, stack
    LMNT_OP_SIN,
    LMNT_OP_COS,
    LMNT_OP_TAN,
    LMNT_OP_ASIN,
    LMNT_OP_ACOS,
    LMNT_OP_ATAN,
    // trig: stack, stack, stack
    LMNT_OP_ATAN2,
    LMNT_OP_SINCOS,
    // pow: stack, stack, stack
    LMNT_OP_POWSS,
    LMNT_OP_POWVV,
    LMNT_OP_POWVS,
    // sqrt: stack, null, stack
    LMNT_OP_SQRTS,
    LMNT_OP_SQRTV,
    // abs: stack, null, stack
    LMNT_OP_ABSS,
    LMNT_OP_ABSV,
    // sum: stack, null, stack
    LMNT_OP_SUMV,
    // min/max: stack, stack, stack
    LMNT_OP_MINSS,
    LMNT_OP_MINVV,
    LMNT_OP_MAXSS,
    LMNT_OP_MAXVV,
    // min/max: stack, null, stack
    LMNT_OP_MINVS,
    LMNT_OP_MAXVS,
    // rounding: stack, null, stack
    LMNT_OP_FLOORS,
    LMNT_OP_FLOORV,
    LMNT_OP_ROUNDS,
    LMNT_OP_ROUNDV,
    LMNT_OP_CEILS,
    LMNT_OP_CEILV,
    // indexing: stackref, immediate, stack
    LMNT_OP_INDEXDIS,
    // indexing: stack, stack, stack
    LMNT_OP_INDEXSSS,
    // indexing: stackref, stack, stack
    LMNT_OP_INDEXDSS,
    // indexing: stackref, stack, stackref
    LMNT_OP_INDEXDSD,
    // extern call: deflo, defhi, imm
    LMNT_OP_EXTCALL,
    // placeholder end operation
    LMNT_OP_END,
};

typedef enum
{
    LMNT_OPERAND_UNUSED,
    LMNT_OPERAND_STACK1,  // S
    LMNT_OPERAND_STACK4,  // V
    LMNT_OPERAND_STACKN,
    LMNT_OPERAND_IMM,     // I
    LMNT_OPERAND_DYNAMIC, // D
    LMNT_OPERAND_DEFPTR,
} lmnt_operand_type;

typedef struct
{
    const char* name;
    lmnt_operand_type operand1;
    lmnt_operand_type operand2;
    lmnt_operand_type operand3;
} lmnt_op_info;

extern const lmnt_op_info lmnt_opcode_info[LMNT_OP_END];

#define LMNT_OP16(a) (char)(a & 0xFF), (char)((a >> 8) & 0xFF)
#define LMNT_OP_BYTES(op, arg1, arg2, arg3) LMNT_OP16(op), LMNT_OP16(arg1), LMNT_OP16(arg2), LMNT_OP16(arg3)

#endif