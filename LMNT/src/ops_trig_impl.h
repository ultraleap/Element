#ifndef LMNT_OPS_TRIG_IMPL_H
#define LMNT_OPS_TRIG_IMPL_H

#include "lmnt/config.h"
#include "lmnt/interpreter.h"
#include <math.h>

#if !defined(LMNT_INLINE_OP)
#define LMNT_INLINE_OP
#endif


LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_sinr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = sinf(ctx->stack[arg1] * ((float)(M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_cosr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = cosf(ctx->stack[arg1] * ((float)(M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_tanr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = tanf(ctx->stack[arg1] * ((float)(M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_asinr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = asinf(ctx->stack[arg1]) * ((float)(1. / (M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_acosr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = acosf(ctx->stack[arg1]) * ((float)(1. / (M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_atanr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = atanf(ctx->stack[arg1]) * ((float)(1. / (M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_atan2r(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = atan2f(ctx->stack[arg1], ctx->stack[arg2]) * ((float)(1. / (M_PI * 2.)));
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_sincosr(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
#if defined(LMNT_SINCOSF)
    LMNT_SINCOSF(ctx->stack[arg1], &(ctx->stack[arg2]), &(ctx->stack[arg3]));
#else
    ctx->stack[arg2] = sinf(ctx->stack[arg1] * ((float)(M_PI * 2.)));
    ctx->stack[arg3] = cosf(ctx->stack[arg1] * ((float)(M_PI * 2.)));
#endif
    return LMNT_OK;
}

#endif