#ifndef LMNT_OPS_BRANCH_IMPL_H
#define LMNT_OPS_BRANCH_IMPL_H

#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "lmnt/common.h"
#include "lmnt/interpreter.h"

#if !defined(LMNT_INLINE_OP)
#define LMNT_INLINE_OP
#endif


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_return(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    return LMNT_RETURNING;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_cmp(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->status_flags &= ~(LMNT_ISTATUS_CMP_EQ | LMNT_ISTATUS_CMP_LT | LMNT_ISTATUS_CMP_GT | LMNT_ISTATUS_CMP_UN);
    ctx->status_flags |= (ctx->stack[arg1] == ctx->stack[arg2]) * LMNT_ISTATUS_CMP_EQ;
    ctx->status_flags |= (ctx->stack[arg1] <  ctx->stack[arg2]) * LMNT_ISTATUS_CMP_LT;
    ctx->status_flags |= (ctx->stack[arg1] >  ctx->stack[arg2]) * LMNT_ISTATUS_CMP_GT;
    ctx->status_flags |= (isnan(ctx->stack[arg1]) || isnan(ctx->stack[arg2])) * LMNT_ISTATUS_CMP_UN;
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_cmpz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->status_flags &= ~(LMNT_ISTATUS_CMP_EQ | LMNT_ISTATUS_CMP_LT | LMNT_ISTATUS_CMP_GT | LMNT_ISTATUS_CMP_UN);
    ctx->status_flags |= (ctx->stack[arg1] == 0.0) * LMNT_ISTATUS_CMP_EQ;
    ctx->status_flags |= (ctx->stack[arg1] <  0.0) * LMNT_ISTATUS_CMP_LT;
    ctx->status_flags |= (ctx->stack[arg1] >  0.0) * LMNT_ISTATUS_CMP_GT;
    ctx->status_flags |= (isnan(ctx->stack[arg1])) * LMNT_ISTATUS_CMP_UN;
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branch(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
    return LMNT_BRANCHING;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchceq(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & LMNT_ISTATUS_CMP_EQ) != 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchcne(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & LMNT_ISTATUS_CMP_EQ) == 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchclt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & LMNT_ISTATUS_CMP_LT) != 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchcle(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & (LMNT_ISTATUS_CMP_LT | LMNT_ISTATUS_CMP_EQ)) != 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchcgt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & LMNT_ISTATUS_CMP_GT) != 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchcge(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & (LMNT_ISTATUS_CMP_GT | LMNT_ISTATUS_CMP_EQ)) != 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchcun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if ((ctx->status_flags & LMNT_ISTATUS_CMP_UN) != 0) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (fabsf(ctx->stack[arg1]) == 0.0f) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchnz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (!isnan(ctx->stack[arg1]) && fabsf(ctx->stack[arg1]) != 0.0f) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchpos(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (!signbit(ctx->stack[arg1])) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchneg(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (signbit(ctx->stack[arg1])) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_branchun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (isnan(ctx->stack[arg1])) {
        ctx->cur_instr = LMNT_COMBINE_OFFSET(arg2, arg3);
        return LMNT_BRANCHING;
    } else {
        return LMNT_OK;
    }
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignceq(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = (ctx->status_flags & LMNT_ISTATUS_CMP_EQ) ? true_val : false_val;
    return LMNT_OK;
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigncne(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = !(ctx->status_flags & LMNT_ISTATUS_CMP_EQ) ? true_val : false_val;
    return LMNT_OK;
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignclt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = (ctx->status_flags & LMNT_ISTATUS_CMP_LT) ? true_val : false_val;
    return LMNT_OK;
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigncle(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = (ctx->status_flags & (LMNT_ISTATUS_CMP_LT | LMNT_ISTATUS_CMP_EQ)) ? true_val : false_val;
    return LMNT_OK;
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigncgt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = (ctx->status_flags & LMNT_ISTATUS_CMP_GT) ? true_val : false_val;
    return LMNT_OK;
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigncge(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = (ctx->status_flags & (LMNT_ISTATUS_CMP_GT | LMNT_ISTATUS_CMP_EQ)) ? true_val : false_val;
    return LMNT_OK;
}


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigncun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_value true_val = (lmnt_value)((int16_t)arg1);
    const lmnt_value false_val = (lmnt_value)((int16_t)arg2);
    ctx->stack[arg3] = (ctx->status_flags & LMNT_ISTATUS_CMP_UN) ? true_val : false_val;
    return LMNT_OK;
}

#endif