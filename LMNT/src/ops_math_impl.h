#ifndef LMNT_OPS_MATH_IMPL_H
#define LMNT_OPS_MATH_IMPL_H

#include <math.h>
#include "lmnt/interpreter.h"

#if !defined(LMNT_INLINE_OP)
#define LMNT_INLINE_OP
#endif


LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_addss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1] + ctx->stack[arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_addvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1 + 0] + ctx->stack[arg2 + 0];
    ctx->stack[arg3 + 1] = ctx->stack[arg1 + 1] + ctx->stack[arg2 + 1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1 + 2] + ctx->stack[arg2 + 2];
    ctx->stack[arg3 + 3] = ctx->stack[arg1 + 3] + ctx->stack[arg2 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_subss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1] - ctx->stack[arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_subvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1 + 0] - ctx->stack[arg2 + 0];
    ctx->stack[arg3 + 1] = ctx->stack[arg1 + 1] - ctx->stack[arg2 + 1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1 + 2] - ctx->stack[arg2 + 2];
    ctx->stack[arg3 + 3] = ctx->stack[arg1 + 3] - ctx->stack[arg2 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_mulss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1] * ctx->stack[arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_mulvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1 + 0] * ctx->stack[arg2 + 0];
    ctx->stack[arg3 + 1] = ctx->stack[arg1 + 1] * ctx->stack[arg2 + 1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1 + 2] * ctx->stack[arg2 + 2];
    ctx->stack[arg3 + 3] = ctx->stack[arg1 + 3] * ctx->stack[arg2 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_divss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1] / ctx->stack[arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_divvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1 + 0] / ctx->stack[arg2 + 0];
    ctx->stack[arg3 + 1] = ctx->stack[arg1 + 1] / ctx->stack[arg2 + 1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1 + 2] / ctx->stack[arg2 + 2];
    ctx->stack[arg3 + 3] = ctx->stack[arg1 + 3] / ctx->stack[arg2 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_modss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = fmodf(ctx->stack[arg1], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_modvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = fmodf(ctx->stack[arg1 + 0], ctx->stack[arg2 + 0]);
    ctx->stack[arg3 + 1] = fmodf(ctx->stack[arg1 + 1], ctx->stack[arg2 + 1]);
    ctx->stack[arg3 + 2] = fmodf(ctx->stack[arg1 + 2], ctx->stack[arg2 + 2]);
    ctx->stack[arg3 + 3] = fmodf(ctx->stack[arg1 + 3], ctx->stack[arg2 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_powss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = powf(ctx->stack[arg1], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_powvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = powf(ctx->stack[arg1 + 0], ctx->stack[arg2 + 0]);
    ctx->stack[arg3 + 1] = powf(ctx->stack[arg1 + 1], ctx->stack[arg2 + 1]);
    ctx->stack[arg3 + 2] = powf(ctx->stack[arg1 + 2], ctx->stack[arg2 + 2]);
    ctx->stack[arg3 + 3] = powf(ctx->stack[arg1 + 3], ctx->stack[arg2 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_powvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = powf(ctx->stack[arg1 + 0], ctx->stack[arg2]);
    ctx->stack[arg3 + 1] = powf(ctx->stack[arg1 + 1], ctx->stack[arg2]);
    ctx->stack[arg3 + 2] = powf(ctx->stack[arg1 + 2], ctx->stack[arg2]);
    ctx->stack[arg3 + 3] = powf(ctx->stack[arg1 + 3], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_sqrts(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = sqrtf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_sqrtv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = sqrtf(ctx->stack[arg1 + 0]);
    ctx->stack[arg3 + 1] = sqrtf(ctx->stack[arg1 + 1]);
    ctx->stack[arg3 + 2] = sqrtf(ctx->stack[arg1 + 2]);
    ctx->stack[arg3 + 3] = sqrtf(ctx->stack[arg1 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_ln(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = logf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_log2(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = log2f(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_log10(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = log10f(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_sumv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1 + 0] + ctx->stack[arg1 + 1] + ctx->stack[arg1 + 2] + ctx->stack[arg1 + 3];
    return LMNT_OK;
}

#endif