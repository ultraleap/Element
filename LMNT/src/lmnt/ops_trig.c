#include "lmnt/ops_trig.h"
#if defined(__GNUC__)
    #define _GNU_SOURCE
    #define USE_GNU_SINCOS
#endif
#include <math.h>

LMNT_ATTR_FAST lmnt_result lmnt_op_sin(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = sinf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_cos(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = cosf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_tan(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = tanf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_asin(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = asinf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_acos(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = acosf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_atan(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = atanf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_atan2(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = atan2f(ctx->stack[arg1], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_sincos(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
#if defined(USE_GNU_SINCOS)
    sincos(ctx->stack[arg1], &(ctx->stack[arg2]), &(ctx->stack[arg3]));
#else
    ctx->stack[arg2] = sinf(ctx->stack[arg1]);
    ctx->stack[arg3] = cosf(ctx->stack[arg1]);
#endif
    return LMNT_OK;
}