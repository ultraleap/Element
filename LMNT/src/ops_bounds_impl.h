#ifndef LMNT_OPS_BOUNDS_IMPL_H
#define LMNT_OPS_BOUNDS_IMPL_H

#include <math.h>

#if !defined(LMNT_INLINE_OP)
#define LMNT_INLINE_OP
#endif

#define MIN(a, b) ((b) < (a) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))


LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_abss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = fabsf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_absv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = fabsf(ctx->stack[arg1 + 0]);
    ctx->stack[arg3 + 1] = fabsf(ctx->stack[arg1 + 1]);
    ctx->stack[arg3 + 2] = fabsf(ctx->stack[arg1 + 2]);
    ctx->stack[arg3 + 3] = fabsf(ctx->stack[arg1 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_floors(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = floorf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_floorv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = floorf(ctx->stack[arg1 + 0]);
    ctx->stack[arg3 + 1] = floorf(ctx->stack[arg1 + 1]);
    ctx->stack[arg3 + 2] = floorf(ctx->stack[arg1 + 2]);
    ctx->stack[arg3 + 3] = floorf(ctx->stack[arg1 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_rounds(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    // roundf does not use round-to-even, rintf and nearbyintf do
    // we assume nobody's doing weird things with the FP environment
    ctx->stack[arg3] = nearbyintf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_roundv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    // roundf does not use round-to-even, rintf and nearbyintf do
    // we assume nobody's doing weird things with the FP environment
    ctx->stack[arg3 + 0] = nearbyintf(ctx->stack[arg1 + 0]);
    ctx->stack[arg3 + 1] = nearbyintf(ctx->stack[arg1 + 1]);
    ctx->stack[arg3 + 2] = nearbyintf(ctx->stack[arg1 + 2]);
    ctx->stack[arg3 + 3] = nearbyintf(ctx->stack[arg1 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_ceils(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ceilf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_ceilv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ceilf(ctx->stack[arg1 + 0]);
    ctx->stack[arg3 + 1] = ceilf(ctx->stack[arg1 + 1]);
    ctx->stack[arg3 + 2] = ceilf(ctx->stack[arg1 + 2]);
    ctx->stack[arg3 + 3] = ceilf(ctx->stack[arg1 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_truncs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = truncf(ctx->stack[arg1]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_truncv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = truncf(ctx->stack[arg1 + 0]);
    ctx->stack[arg3 + 1] = truncf(ctx->stack[arg1 + 1]);
    ctx->stack[arg3 + 2] = truncf(ctx->stack[arg1 + 2]);
    ctx->stack[arg3 + 3] = truncf(ctx->stack[arg1 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_minss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = MIN(ctx->stack[arg1], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_minvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = MIN(ctx->stack[arg1 + 0], ctx->stack[arg2 + 0]);
    ctx->stack[arg3 + 1] = MIN(ctx->stack[arg1 + 1], ctx->stack[arg2 + 1]);
    ctx->stack[arg3 + 2] = MIN(ctx->stack[arg1 + 2], ctx->stack[arg2 + 2]);
    ctx->stack[arg3 + 3] = MIN(ctx->stack[arg1 + 3], ctx->stack[arg2 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_maxss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = MAX(ctx->stack[arg1], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_maxvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = MAX(ctx->stack[arg1 + 0], ctx->stack[arg2 + 0]);
    ctx->stack[arg3 + 1] = MAX(ctx->stack[arg1 + 1], ctx->stack[arg2 + 1]);
    ctx->stack[arg3 + 2] = MAX(ctx->stack[arg1 + 2], ctx->stack[arg2 + 2]);
    ctx->stack[arg3 + 3] = MAX(ctx->stack[arg1 + 3], ctx->stack[arg2 + 3]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_minvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = MIN(ctx->stack[arg1 + 0], ctx->stack[arg2]);
    ctx->stack[arg3 + 1] = MIN(ctx->stack[arg1 + 1], ctx->stack[arg2]);
    ctx->stack[arg3 + 2] = MIN(ctx->stack[arg1 + 2], ctx->stack[arg2]);
    ctx->stack[arg3 + 3] = MIN(ctx->stack[arg1 + 3], ctx->stack[arg2]);
    return LMNT_OK;
}

LMNT_ATTR_FAST static inline LMNT_INLINE_OP lmnt_result lmnt_op_maxvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = MAX(ctx->stack[arg1 + 0], ctx->stack[arg2]);
    ctx->stack[arg3 + 1] = MAX(ctx->stack[arg1 + 1], ctx->stack[arg2]);
    ctx->stack[arg3 + 2] = MAX(ctx->stack[arg1 + 2], ctx->stack[arg2]);
    ctx->stack[arg3 + 3] = MAX(ctx->stack[arg1 + 3], ctx->stack[arg2]);
    return LMNT_OK;
}

#undef MAX
#undef MIN

#endif