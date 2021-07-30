#ifndef LMNT_DISPATCH_SWITCH_H
#define LMNT_DISPATCH_SWITCH_H

#include <assert.h>

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


LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result execute_function(lmnt_ictx* ctx, const lmnt_code* defcode, const lmnt_instruction* instructions)
{
    lmnt_result opresult = LMNT_OK;
    // Grab the current instruction as a local rather than updating the ctx every time - faster
    lmnt_loffset instr = ctx->cur_instr;
    const lmnt_loffset icount = defcode->instructions_count;

    for (; instr < icount; ++instr)
    {
        const lmnt_instruction op = instructions[instr];
        assert(op.opcode < LMNT_OP_END);
        switch (op.opcode)
        {
        case LMNT_OP_NOOP:      opresult = lmnt_op_noop(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_RETURN:    opresult = lmnt_op_return(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNSS:  opresult = lmnt_op_assignss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNVV:  opresult = lmnt_op_assignvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNSV:  opresult = lmnt_op_assignsv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNIIS: opresult = lmnt_op_assigniis(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNIBS: opresult = lmnt_op_assignibs(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNIIV: opresult = lmnt_op_assigniiv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNIBV: opresult = lmnt_op_assignibv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DLOADIIS:  opresult = lmnt_op_dloadiis(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DLOADIIV:  opresult = lmnt_op_dloadiiv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DLOADIRS:  opresult = lmnt_op_dloadirs(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DLOADIRV:  opresult = lmnt_op_dloadirv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DSECLEN:   opresult = lmnt_op_dseclen(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ADDSS:     opresult = lmnt_op_addss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ADDVV:     opresult = lmnt_op_addvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SUBSS:     opresult = lmnt_op_subss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SUBVV:     opresult = lmnt_op_subvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MULSS:     opresult = lmnt_op_mulss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MULVV:     opresult = lmnt_op_mulvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DIVSS:     opresult = lmnt_op_divss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_DIVVV:     opresult = lmnt_op_divvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MODSS:     opresult = lmnt_op_modss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MODVV:     opresult = lmnt_op_modvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SIN:       opresult = lmnt_op_sin(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_COS:       opresult = lmnt_op_cos(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_TAN:       opresult = lmnt_op_tan(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASIN:      opresult = lmnt_op_asin(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ACOS:      opresult = lmnt_op_acos(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ATAN:      opresult = lmnt_op_atan(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ATAN2:     opresult = lmnt_op_atan2(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SINCOS:    opresult = lmnt_op_sincos(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_POWSS:     opresult = lmnt_op_powss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_POWVV:     opresult = lmnt_op_powvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_POWVS:     opresult = lmnt_op_powvs(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SQRTS:     opresult = lmnt_op_sqrts(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SQRTV:     opresult = lmnt_op_sqrtv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_LOG:       opresult = lmnt_op_log(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_LN:        opresult = lmnt_op_ln(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_LOG2:      opresult = lmnt_op_log2(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_LOG10:     opresult = lmnt_op_log10(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ABSS:      opresult = lmnt_op_abss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ABSV:      opresult = lmnt_op_absv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_SUMV:      opresult = lmnt_op_sumv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MINSS:     opresult = lmnt_op_minss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MINVV:     opresult = lmnt_op_minvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MAXSS:     opresult = lmnt_op_maxss(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MAXVV:     opresult = lmnt_op_maxvv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MINVS:     opresult = lmnt_op_minvs(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_MAXVS:     opresult = lmnt_op_maxvs(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_FLOORS:    opresult = lmnt_op_floors(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_FLOORV:    opresult = lmnt_op_floorv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ROUNDS:    opresult = lmnt_op_rounds(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ROUNDV:    opresult = lmnt_op_roundv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_CEILS:     opresult = lmnt_op_ceils(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_CEILV:     opresult = lmnt_op_ceilv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_TRUNCS:    opresult = lmnt_op_truncs(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_TRUNCV:    opresult = lmnt_op_truncv(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_INDEXRIS:  opresult = lmnt_op_indexris(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_INDEXRIR:  opresult = lmnt_op_indexrir(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCH:    opresult = lmnt_op_branch(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHZ:   opresult = lmnt_op_branchz(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHNZ:  opresult = lmnt_op_branchnz(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHPOS: opresult = lmnt_op_branchpos(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHNEG: opresult = lmnt_op_branchneg(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHUN:  opresult = lmnt_op_branchun(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_CMP:       opresult = lmnt_op_cmp(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_CMPZ:      opresult = lmnt_op_cmpz(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCEQ: opresult = lmnt_op_branchceq(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCNE: opresult = lmnt_op_branchcne(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCLT: opresult = lmnt_op_branchclt(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCLE: opresult = lmnt_op_branchcle(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCGT: opresult = lmnt_op_branchcgt(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCGE: opresult = lmnt_op_branchcge(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_BRANCHCUN: opresult = lmnt_op_branchcun(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNCEQ: opresult = lmnt_op_assignceq(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNCNE: opresult = lmnt_op_assigncne(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNCLT: opresult = lmnt_op_assignclt(ctx, op.arg1, op.arg2, op.arg3); break; 
        case LMNT_OP_ASSIGNCLE: opresult = lmnt_op_assigncle(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNCGT: opresult = lmnt_op_assigncgt(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNCGE: opresult = lmnt_op_assigncge(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_ASSIGNCUN: opresult = lmnt_op_assigncun(ctx, op.arg1, op.arg2, op.arg3); break;
        case LMNT_OP_EXTCALL:   opresult = lmnt_op_extcall(ctx, op.arg1, op.arg2, op.arg3); break;
        default:                LMNT_UNREACHABLE(); opresult = LMNT_ERROR_INTERNAL; break;
        }

#if defined(LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS)
        print_execution_context(ctx, instr, instructions[instr]);
#endif
        if (LMNT_UNLIKELY(opresult | (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))) {
            if (opresult == LMNT_BRANCHING) {
                // the context's instruction pointer has been updated, refresh
                instr = ctx->cur_instr - 1; // will be incremented in the next iteration
                continue;
            } else {
                if (opresult == LMNT_OK && (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))
                    opresult = LMNT_INTERRUPTED;
                break;
            }
        }
    }

    ctx->cur_instr = instr;
    return opresult;
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result interrupt_function(lmnt_ictx* ctx)
{
    ctx->status_flags |= LMNT_ISTATUS_INTERRUPTED;
    return LMNT_OK;
}

#endif