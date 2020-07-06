#include "lmnt/ops_misc.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "lmnt/common.h"
#include "lmnt/config.h"
#include "lmnt/helpers.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_noop(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assignss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1];
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assignvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1 + 0];
    ctx->stack[arg3 + 1] = ctx->stack[arg1 + 1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1 + 2];
    ctx->stack[arg3 + 3] = ctx->stack[arg1 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assignsv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1];
    ctx->stack[arg3 + 1] = ctx->stack[arg1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1];
    ctx->stack[arg3 + 3] = ctx->stack[arg1];
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assigniis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    _Static_assert(sizeof(lmnt_loffset) == sizeof(lmnt_value) && sizeof(lmnt_loffset) == sizeof(int32_t),
        "lmnt_loffset, lmnt_value and int32_t must be the same size or ASSIGNI* must be reworked");
    const lmnt_loffset uval = LMNT_COMBINE_OFFSET(arg1, arg2);
    const int32_t val = *(const int32_t*)(&uval);
    ctx->stack[arg3] = (lmnt_value)val;
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assignibs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    _Static_assert(sizeof(lmnt_offset) * 2 == sizeof(lmnt_value),
        "lmnt_offset * 2 must be the same size as lmnt_value or ASSIGNI* must be reworked");
    _Static_assert(sizeof(lmnt_loffset) == sizeof(lmnt_value),
        "lmnt_loffset must be the same size as lmnt_value or ASSIGNI* must be reworked");
    const lmnt_loffset val = LMNT_COMBINE_OFFSET(arg1, arg2);
    ctx->stack[arg3] = *(lmnt_value*)(&val);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assigniiv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    _Static_assert(sizeof(lmnt_loffset) == sizeof(lmnt_value) && sizeof(lmnt_loffset) == sizeof(int32_t),
        "lmnt_loffset, lmnt_value and int32_t must be the same size or ASSIGNI* must be reworked");
    const lmnt_loffset uval = LMNT_COMBINE_OFFSET(arg1, arg2);
    const int32_t val = *(const int32_t*)(&uval);
    ctx->stack[arg3 + 0] = (lmnt_value)val;
    ctx->stack[arg3 + 1] = (lmnt_value)val;
    ctx->stack[arg3 + 2] = (lmnt_value)val;
    ctx->stack[arg3 + 3] = (lmnt_value)val;
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_assignibv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    _Static_assert(sizeof(lmnt_offset) * 2 == sizeof(lmnt_value),
        "lmnt_offset * 2 must be the same size as lmnt_value or ASSIGNI* must be reworked");
    _Static_assert(sizeof(lmnt_loffset) == sizeof(lmnt_value),
        "lmnt_loffset must be the same size as lmnt_value or ASSIGNI* must be reworked");
    const lmnt_loffset val = LMNT_COMBINE_OFFSET(arg1, arg2);
    ctx->stack[arg3 + 0] = *(lmnt_value*)(&val);
    ctx->stack[arg3 + 1] = *(lmnt_value*)(&val);
    ctx->stack[arg3 + 2] = *(lmnt_value*)(&val);
    ctx->stack[arg3 + 3] = *(lmnt_value*)(&val);
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_indexdis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (isnan(ctx->stack[arg1]) || isinf(ctx->stack[arg1])) return LMNT_ERROR_ACCESS_VIOLATION;
    size_t arg1v = value_to_size_t(ctx->stack[arg1]);
    if (arg1v + arg2 >= ctx->cur_stack_count) return LMNT_ERROR_ACCESS_VIOLATION;
    ctx->stack[arg3] = ctx->stack[arg1v + arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_indexdid(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (isnan(ctx->stack[arg1]) || isinf(ctx->stack[arg1])) return LMNT_ERROR_ACCESS_VIOLATION;
    if (isnan(ctx->stack[arg3]) || isinf(ctx->stack[arg3])) return LMNT_ERROR_ACCESS_VIOLATION;
    size_t arg1v = value_to_size_t(ctx->stack[arg1]);
    size_t arg3v = value_to_size_t(ctx->stack[arg3]);
    if (arg1v + arg2 >= ctx->cur_stack_count || arg3v >= ctx->cur_stack_count) return LMNT_ERROR_ACCESS_VIOLATION;
    ctx->stack[arg3v] = ctx->stack[arg1v + arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST lmnt_result lmnt_op_interrupt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    return LMNT_INTERRUPTED;
}

