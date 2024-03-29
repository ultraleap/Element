#include "lmnt/opcodes.h"

#include "lmnt/interpreter.h"

const lmnt_op_info lmnt_opcode_info[LMNT_OP_END] = {
    { "NOOP",      LMNT_OPERAND_UNUSED,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_UNUSED   },
    { "RETURN",    LMNT_OPERAND_UNUSED,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_UNUSED   },
    { "ASSIGNSS",  LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ASSIGNVV",  LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "ASSIGNSV",  LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "ASSIGNIBS", LMNT_OPERAND_IMM,      LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK1   },
    { "ASSIGNIBV", LMNT_OPERAND_IMM,      LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK4   },
    { "DLOADIIS",  LMNT_OPERAND_IMM,      LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK1   },
    { "DLOADIIV",  LMNT_OPERAND_IMM,      LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK4   },
    { "DLOADIRS",  LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "DLOADIRV",  LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK4   },
    { "DSECLEN",   LMNT_OPERAND_IMM,      LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ADDSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ADDVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "SUBSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "SUBVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "MULSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "MULVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "DIVSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "DIVVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "REMSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "REMVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "SIN",       LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "COS",       LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "TAN",       LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ASIN",      LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ACOS",      LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ATAN",      LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ATAN2",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "SINCOS",    LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "POWSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "POWVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "POWVS",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK4   },
    { "SQRTS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "SQRTV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "LN",        LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "LOG2",      LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "LOG10",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ABSS",      LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ABSV",      LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "SUMV",      LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "MINSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "MINVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "MAXSS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "MAXVV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK4   },
    { "MINVS",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK4   },
    { "MAXVS",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK4   },
    { "FLOORS",    LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "FLOORV",    LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "ROUNDS",    LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "ROUNDV",    LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "CEILS",     LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "CEILV",     LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "TRUNCS",    LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK1   },
    { "TRUNCV",    LMNT_OPERAND_STACK4,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_STACK4   },
    { "INDEXRIS",  LMNT_OPERAND_STACKREF, LMNT_OPERAND_IMM,      LMNT_OPERAND_STACK1   },
    { "INDEXRIR",  LMNT_OPERAND_STACKREF, LMNT_OPERAND_IMM,      LMNT_OPERAND_STACKREF },
    { "BRANCH",    LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHZ",   LMNT_OPERAND_STACK1,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHNZ",  LMNT_OPERAND_STACK1,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHPOS", LMNT_OPERAND_STACK1,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHNEG", LMNT_OPERAND_STACK1,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHUN",  LMNT_OPERAND_STACK1,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "CMP",       LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED   },
    { "CMPZ",      LMNT_OPERAND_STACK1,   LMNT_OPERAND_UNUSED,   LMNT_OPERAND_UNUSED   },
    { "BRANCHCEQ", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHCNE", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHCLT", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHCLE", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHCGT", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHCGE", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "BRANCHCUN", LMNT_OPERAND_UNUSED,   LMNT_OPERAND_CODEPTR,  LMNT_OPERAND_CODEPTR  },
    { "ASSIGNCEQ", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ASSIGNCNE", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ASSIGNCLT", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ASSIGNCLE", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ASSIGNCGT", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ASSIGNCGE", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "ASSIGNCUN", LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1,   LMNT_OPERAND_STACK1   },
    { "EXTCALL",   LMNT_OPERAND_DEFPTR,   LMNT_OPERAND_DEFPTR,   LMNT_OPERAND_STACKN   },
};

const lmnt_op_info* lmnt_get_opcode_info(lmnt_opcode op)
{
    return LMNT_LIKELY(op < LMNT_OP_END) ? &(lmnt_opcode_info[op]) : NULL;
}