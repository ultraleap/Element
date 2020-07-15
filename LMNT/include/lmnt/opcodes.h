#ifndef LMNT_OPCODES_H
#define LMNT_OPCODES_H

#include <stdint.h>

typedef uint16_t lmnt_opcode;

enum
{
    // no-op: null, null, null
    LMNT_OP_NOOP = 0,
    LMNT_OP_RETURN,
    // assign variants: stack, null, stack
    LMNT_OP_ASSIGNSS,
    LMNT_OP_ASSIGNVV,
    LMNT_OP_ASSIGNSV,
    // assign variants: immlo, immhi, stack
    LMNT_OP_ASSIGNIIS,
    LMNT_OP_ASSIGNIBS,
    LMNT_OP_ASSIGNIIV,
    LMNT_OP_ASSIGNIBV,
    // data load: immediate, immediate, stack
    LMNT_OP_DLOADIIS,
    LMNT_OP_DLOADIIV,
    // data load: immediate, data ref, stack
    LMNT_OP_DLOADIRS,
    LMNT_OP_DLOADIRV,
    // data section get count: immediate, null, stack
    LMNT_OP_DSECLEN,
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
    LMNT_OP_TRUNCS,
    LMNT_OP_TRUNCV,
    // indexing: stackref, immediate, stack
    LMNT_OP_INDEXRIS,
    // indexing: stackref, immediate, stackref
    LMNT_OP_INDEXRIR,
    // compare: stack, stack, null
    LMNT_OP_CMP,
    // branch: null, codelo, codehi
    LMNT_OP_BRANCH,
    LMNT_OP_BRANCHCEQ,
    LMNT_OP_BRANCHCNE,
    LMNT_OP_BRANCHCLT,
    LMNT_OP_BRANCHCLE,
    LMNT_OP_BRANCHCGT,
    LMNT_OP_BRANCHCGE,
    LMNT_OP_BRANCHCUN,
    // branch: stack, codelo, codehi
    LMNT_OP_BRANCHZ,
    LMNT_OP_BRANCHNZ,
    LMNT_OP_BRANCHPOS,
    LMNT_OP_BRANCHNEG,
    LMNT_OP_BRANCHUN,
    // extern call: deflo, defhi, stack
    LMNT_OP_EXTCALL,
    // placeholder end operation
    LMNT_OP_END,
};

#define LMNT_IS_BRANCH_OP(opcode) ((opcode) >= LMNT_OP_BRANCH && (opcode) <= LMNT_OP_BRANCHUN)


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

extern const lmnt_op_info lmnt_opcode_info[LMNT_OP_END];

#define LMNT_OP16(a) (char)(a & 0xFF), (char)((a >> 8) & 0xFF)
#define LMNT_OP_BYTES(op, arg1, arg2, arg3) LMNT_OP16(op), LMNT_OP16(arg1), LMNT_OP16(arg2), LMNT_OP16(arg3)

#endif