#ifndef LMNT_DISPATCH_COMPUTED_GOTO_H
#define LMNT_DISPATCH_COMPUTED_GOTO_H

#include "lmnt/config.h"
#include "lmnt/common.h"
#include "lmnt/interpreter.h"

#define LMNT_INLINE_OP LMNT_FORCEINLINE
#include "ops_bounds_impl.h"
#include "ops_branch_impl.h"
#include "ops_fncall_impl.h"
#include "ops_math_impl.h"
#include "ops_misc_impl.h"
#include "ops_trig_impl.h"

static const char* const dispatch_method(void)
{
    return "computed gotos";
}

// cannot be inlined due to static table
LMNT_ATTR_FAST static lmnt_result execute_function(lmnt_ictx* ctx, const lmnt_code* defcode, const lmnt_instruction* instructions)
{
    // set up table
    static void* const jump_targets[LMNT_OP_END] = {
        &&op_noop,
        &&op_return,
        &&op_assignss,
        &&op_assignvv,
        &&op_assignsv,
        &&op_assigniis,
        &&op_assignibs,
        &&op_assigniiv,
        &&op_assignibv,
        &&op_dloadiis,
        &&op_dloadiiv,
        &&op_dloadirs,
        &&op_dloadirv,
        &&op_dseclen,
        &&op_addss,
        &&op_addvv,
        &&op_subss,
        &&op_subvv,
        &&op_mulss,
        &&op_mulvv,
        &&op_divss,
        &&op_divvv,
        &&op_modss,
        &&op_modvv,
        &&op_sin,
        &&op_cos,
        &&op_tan,
        &&op_asin,
        &&op_acos,
        &&op_atan,
        &&op_atan2,
        &&op_sincos,
        &&op_powss,
        &&op_powvv,
        &&op_powvs,
        &&op_sqrts,
        &&op_sqrtv,
        &&op_log,
        &&op_ln,
        &&op_log2,
        &&op_log10,
        &&op_abss,
        &&op_absv,
        &&op_sumv,
        &&op_minss,
        &&op_minvv,
        &&op_maxss,
        &&op_maxvv,
        &&op_minvs,
        &&op_maxvs,
        &&op_floors,
        &&op_floorv,
        &&op_rounds,
        &&op_roundv,
        &&op_ceils,
        &&op_ceilv,
        &&op_truncs,
        &&op_truncv,
        &&op_indexris,
        &&op_indexrir,
        &&op_branch,
        &&op_branchz,
        &&op_branchnz,
        &&op_branchpos,
        &&op_branchneg,
        &&op_branchun,
        &&op_cmp,
        &&op_cmpz,
        &&op_branchceq,
        &&op_branchcne,
        &&op_branchclt,
        &&op_branchcle,
        &&op_branchcgt,
        &&op_branchcge,
        &&op_branchcun,
        &&op_assignceq,
        &&op_assigncne,
        &&op_assignclt,
        &&op_assigncle,
        &&op_assigncgt,
        &&op_assigncge,
        &&op_assigncun,
        &&op_extcall,
    };

    lmnt_result opresult = LMNT_OK;
    // Grab the current instruction as a local rather than updating the ctx every time - faster
    lmnt_loffset instr = ctx->cur_instr - 1; // incremented at start of dispatch
    const lmnt_loffset icount = defcode->instructions_count;
    lmnt_instruction op;

dispatch:
    if (LMNT_UNLIKELY(opresult | (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))) {
        if (opresult == LMNT_BRANCHING) {
            // the context's instruction pointer has been updated, refresh
            instr = ctx->cur_instr - 1; // will be incremented momentarily
        } else {
            if (opresult == LMNT_OK && (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))
                opresult = LMNT_INTERRUPTED;
            goto end;
        }
    }
dispatchok:
#if defined(LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS)
    print_execution_context(ctx, instr, instructions[instr]);
#endif
    if (++instr >= icount) {
        goto end;
    }

    op = instructions[instr];
    goto *jump_targets[op.opcode];

#define GENERATE_OP(name, nextstop) \
op_##name: \
    opresult = lmnt_op_##name(ctx, op.arg1, op.arg2, op.arg3); \
    goto nextstop;
#define GENERATE_OP_NOFAIL(name) \
op_##name: \
    lmnt_op_##name(ctx, op.arg1, op.arg2, op.arg3); \
    goto dispatchok;

op_noop:
    goto dispatch;
op_return:
    opresult = LMNT_RETURNING;
    goto end;
GENERATE_OP_NOFAIL(assignss);
GENERATE_OP_NOFAIL(assignvv);
GENERATE_OP_NOFAIL(assignsv);
GENERATE_OP_NOFAIL(assigniis);
GENERATE_OP_NOFAIL(assignibs);
GENERATE_OP_NOFAIL(assigniiv);
GENERATE_OP_NOFAIL(assignibv);
GENERATE_OP(dloadiis, dispatch);
GENERATE_OP(dloadiiv, dispatch);
GENERATE_OP(dloadirs, dispatch);
GENERATE_OP(dloadirv, dispatch);
GENERATE_OP_NOFAIL(dseclen);
GENERATE_OP_NOFAIL(addss);
GENERATE_OP_NOFAIL(addvv);
GENERATE_OP_NOFAIL(subss);
GENERATE_OP_NOFAIL(subvv);
GENERATE_OP_NOFAIL(mulss);
GENERATE_OP_NOFAIL(mulvv);
GENERATE_OP_NOFAIL(divss);
GENERATE_OP_NOFAIL(divvv);
GENERATE_OP_NOFAIL(modss);
GENERATE_OP_NOFAIL(modvv);
GENERATE_OP_NOFAIL(sin);
GENERATE_OP_NOFAIL(cos);
GENERATE_OP_NOFAIL(tan);
GENERATE_OP_NOFAIL(asin);
GENERATE_OP_NOFAIL(acos);
GENERATE_OP_NOFAIL(atan);
GENERATE_OP_NOFAIL(atan2);
GENERATE_OP_NOFAIL(sincos);
GENERATE_OP_NOFAIL(powss);
GENERATE_OP_NOFAIL(powvv);
GENERATE_OP_NOFAIL(powvs);
GENERATE_OP_NOFAIL(sqrts);
GENERATE_OP_NOFAIL(sqrtv);
GENERATE_OP_NOFAIL(log);
GENERATE_OP_NOFAIL(ln);
GENERATE_OP_NOFAIL(log2);
GENERATE_OP_NOFAIL(log10);
GENERATE_OP_NOFAIL(abss);
GENERATE_OP_NOFAIL(absv);
GENERATE_OP_NOFAIL(sumv);
GENERATE_OP_NOFAIL(minss);
GENERATE_OP_NOFAIL(minvv);
GENERATE_OP_NOFAIL(maxss);
GENERATE_OP_NOFAIL(maxvv);
GENERATE_OP_NOFAIL(minvs);
GENERATE_OP_NOFAIL(maxvs);
GENERATE_OP_NOFAIL(floors);
GENERATE_OP_NOFAIL(floorv);
GENERATE_OP_NOFAIL(rounds);
GENERATE_OP_NOFAIL(roundv);
GENERATE_OP_NOFAIL(ceils);
GENERATE_OP_NOFAIL(ceilv);
GENERATE_OP_NOFAIL(truncs);
GENERATE_OP_NOFAIL(truncv);
GENERATE_OP(indexris, dispatch);
GENERATE_OP(indexrir, dispatch);
GENERATE_OP(branch, branchcheck);
GENERATE_OP(branchz, branchcheck);
GENERATE_OP(branchnz, branchcheck);
GENERATE_OP(branchpos, branchcheck);
GENERATE_OP(branchneg, branchcheck);
GENERATE_OP(branchun, branchcheck);
GENERATE_OP_NOFAIL(cmp);
GENERATE_OP_NOFAIL(cmpz);
GENERATE_OP(branchceq, branchcheck);
GENERATE_OP(branchcne, branchcheck);
GENERATE_OP(branchclt, branchcheck);
GENERATE_OP(branchcle, branchcheck);
GENERATE_OP(branchcgt, branchcheck);
GENERATE_OP(branchcge, branchcheck);
GENERATE_OP(branchcun, branchcheck);
GENERATE_OP_NOFAIL(assignceq);
GENERATE_OP_NOFAIL(assigncne);
GENERATE_OP_NOFAIL(assignclt);
GENERATE_OP_NOFAIL(assigncle);
GENERATE_OP_NOFAIL(assigncgt);
GENERATE_OP_NOFAIL(assigncge);
GENERATE_OP_NOFAIL(assigncun);
GENERATE_OP(extcall, dispatch);

#undef GENERATE_OP_NOFAIL
#undef GENERATE_OP

end:
    ctx->cur_instr = instr;
    return opresult;

branchcheck:
    if (opresult == LMNT_BRANCHING) {
        // the context's instruction pointer has been updated, refresh
        instr = ctx->cur_instr - 1; // will be incremented momentarily
        opresult = LMNT_OK;
    }
    goto dispatch;
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result interrupt_function(lmnt_ictx* ctx)
{
    ctx->status_flags |= LMNT_ISTATUS_INTERRUPTED;
    return LMNT_OK;
}

#endif