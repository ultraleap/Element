#ifndef LMNT_DISPATCH_JUMPTABLE_H
#define LMNT_DISPATCH_JUMPTABLE_H

#include <assert.h>

#include "lmnt/config.h"
#include "lmnt/common.h"
#include "lmnt/interpreter.h"

#include "ops_bounds_impl.h"
#include "ops_branch_impl.h"
#include "ops_fncall_impl.h"
#include "ops_math_impl.h"
#include "ops_misc_impl.h"
#include "ops_trig_impl.h"

static const char* const dispatch_method(void)
{
    return "jump table";
}

LMNT_ATTR_FAST static lmnt_op_fn lmnt_op_functions[LMNT_OP_END] = {
    lmnt_op_noop,
    lmnt_op_return,
    lmnt_op_assignss,
    lmnt_op_assignvv,
    lmnt_op_assignsv,
    lmnt_op_assigniis,
    lmnt_op_assignibs,
    lmnt_op_assigniiv,
    lmnt_op_assignibv,
    lmnt_op_dloadiis,
    lmnt_op_dloadiiv,
    lmnt_op_dloadirs,
    lmnt_op_dloadirv,
    lmnt_op_dseclen,
    lmnt_op_addss,
    lmnt_op_addvv,
    lmnt_op_subss,
    lmnt_op_subvv,
    lmnt_op_mulss,
    lmnt_op_mulvv,
    lmnt_op_divss,
    lmnt_op_divvv,
    lmnt_op_modss,
    lmnt_op_modvv,
    lmnt_op_sin,
    lmnt_op_cos,
    lmnt_op_tan,
    lmnt_op_asin,
    lmnt_op_acos,
    lmnt_op_atan,
    lmnt_op_atan2,
    lmnt_op_sincos,
    lmnt_op_powss,
    lmnt_op_powvv,
    lmnt_op_powvs,
    lmnt_op_sqrts,
    lmnt_op_sqrtv,
    lmnt_op_log,
    lmnt_op_ln,
    lmnt_op_log2,
    lmnt_op_log10,
    lmnt_op_abss,
    lmnt_op_absv,
    lmnt_op_sumv,
    lmnt_op_minss,
    lmnt_op_minvv,
    lmnt_op_maxss,
    lmnt_op_maxvv,
    lmnt_op_minvs,
    lmnt_op_maxvs,
    lmnt_op_floors,
    lmnt_op_floorv,
    lmnt_op_rounds,
    lmnt_op_roundv,
    lmnt_op_ceils,
    lmnt_op_ceilv,
    lmnt_op_truncs,
    lmnt_op_truncv,
    lmnt_op_indexris,
    lmnt_op_indexrir,
    lmnt_op_branch,
    lmnt_op_branchz,
    lmnt_op_branchnz,
    lmnt_op_branchpos,
    lmnt_op_branchneg,
    lmnt_op_branchun,
    lmnt_op_cmp,
    lmnt_op_cmpz,
    lmnt_op_branchceq,
    lmnt_op_branchcne,
    lmnt_op_branchclt,
    lmnt_op_branchcle,
    lmnt_op_branchcgt,
    lmnt_op_branchcge,
    lmnt_op_branchcun,
    lmnt_op_assignceq,
    lmnt_op_assigncne,
    lmnt_op_assignclt,
    lmnt_op_assigncle,
    lmnt_op_assigncgt,
    lmnt_op_assigncge,
    lmnt_op_assigncun,
    lmnt_op_extcall,
};

static lmnt_op_fn lmnt_interrupt_functions[LMNT_OP_END] = {
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
    lmnt_op_interrupt,
};

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result execute_instruction(lmnt_ictx* ctx, const lmnt_instruction op)
{
    assert(op.opcode < LMNT_OP_END);
    assert(ctx->op_functions[op.opcode]);
    return ctx->op_functions[op.opcode](ctx, op.arg1, op.arg2, op.arg3);
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result execute_function(lmnt_ictx* ctx, const lmnt_code* defcode, const lmnt_instruction* instructions)
{
    lmnt_result opresult = LMNT_OK;
    // Make sure the context is set up to use the real ops
    ctx->op_functions = lmnt_op_functions;
    // Grab the current instruction as a local rather than updating the ctx every time - faster
    lmnt_loffset instr;
    const lmnt_loffset icount = defcode->instructions_count;
    for (instr = ctx->cur_instr; instr < icount; ++instr)
    {
        opresult = execute_instruction(ctx, instructions[instr]);
#if defined(LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS)
        print_execution_context(ctx, instr, instructions[instr]);
#endif
        if (LMNT_UNLIKELY(opresult != LMNT_OK)) {
            if (opresult == LMNT_BRANCHING) {
                // the context's instruction pointer has been updated, refresh
                instr = ctx->cur_instr - 1; // will be incremented by loop
                continue;
            } else {
                break;
            }
        }
    }
    ctx->cur_instr = instr;
    return opresult;
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result interrupt_function(lmnt_ictx* ctx)
{
    ctx->op_functions = lmnt_interrupt_functions;
    return LMNT_OK;
}

#endif