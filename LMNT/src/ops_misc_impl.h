#ifndef LMNT_OPS_MISC_IMPL_H
#define LMNT_OPS_MISC_IMPL_H

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "lmnt/common.h"
#include "lmnt/config.h"
#include "lmnt/interpreter.h"
#include "helpers.h"

#if !defined(LMNT_INLINE_OP)
#define LMNT_INLINE_OP
#endif


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_noop(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3] = ctx->stack[arg1];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1 + 0];
    ctx->stack[arg3 + 1] = ctx->stack[arg1 + 1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1 + 2];
    ctx->stack[arg3 + 3] = ctx->stack[arg1 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignsv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    ctx->stack[arg3 + 0] = ctx->stack[arg1];
    ctx->stack[arg3 + 1] = ctx->stack[arg1];
    ctx->stack[arg3 + 2] = ctx->stack[arg1];
    ctx->stack[arg3 + 3] = ctx->stack[arg1];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigniis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    LMNT_STATIC_ASSERT(sizeof(lmnt_loffset) == sizeof(lmnt_value) && sizeof(lmnt_loffset) == sizeof(int32_t),
        "lmnt_loffset, lmnt_value and int32_t must be the same size or ASSIGNI* must be reworked");
    const lmnt_loffset uval = LMNT_COMBINE_OFFSET(arg1, arg2);
    const int32_t val = *(const int32_t*)(&uval);
    ctx->stack[arg3] = (lmnt_value)val;
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignibs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    LMNT_STATIC_ASSERT(sizeof(lmnt_offset) * 2 == sizeof(lmnt_value),
        "lmnt_offset * 2 must be the same size as lmnt_value or ASSIGNI* must be reworked");
    LMNT_STATIC_ASSERT(sizeof(lmnt_loffset) == sizeof(lmnt_value),
        "lmnt_loffset must be the same size as lmnt_value or ASSIGNI* must be reworked");
    const lmnt_loffset val = LMNT_COMBINE_OFFSET(arg1, arg2);
    ctx->stack[arg3] = *(lmnt_value*)(&val);
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assigniiv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    LMNT_STATIC_ASSERT(sizeof(lmnt_loffset) == sizeof(lmnt_value) && sizeof(lmnt_loffset) == sizeof(int32_t),
        "lmnt_loffset, lmnt_value and int32_t must be the same size or ASSIGNI* must be reworked");
    const lmnt_loffset uval = LMNT_COMBINE_OFFSET(arg1, arg2);
    const int32_t val = *(const int32_t*)(&uval);
    ctx->stack[arg3 + 0] = (lmnt_value)val;
    ctx->stack[arg3 + 1] = (lmnt_value)val;
    ctx->stack[arg3 + 2] = (lmnt_value)val;
    ctx->stack[arg3 + 3] = (lmnt_value)val;
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_assignibv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    LMNT_STATIC_ASSERT(sizeof(lmnt_offset) * 2 == sizeof(lmnt_value),
        "lmnt_offset * 2 must be the same size as lmnt_value or ASSIGNI* must be reworked");
    LMNT_STATIC_ASSERT(sizeof(lmnt_loffset) == sizeof(lmnt_value),
        "lmnt_loffset must be the same size as lmnt_value or ASSIGNI* must be reworked");
    const lmnt_loffset val = LMNT_COMBINE_OFFSET(arg1, arg2);
    ctx->stack[arg3 + 0] = *(lmnt_value*)(&val);
    ctx->stack[arg3 + 1] = *(lmnt_value*)(&val);
    ctx->stack[arg3 + 2] = *(lmnt_value*)(&val);
    ctx->stack[arg3 + 3] = *(lmnt_value*)(&val);
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_dloadiis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_data_section* sec = validated_get_data_section(&ctx->archive, arg1);
    const lmnt_value* values = validated_get_data_block(&ctx->archive, sec->offset);
    ctx->stack[arg3] = values[arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_dloadiiv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_data_section* sec = validated_get_data_section(&ctx->archive, arg1);
    const lmnt_value* values = validated_get_data_block(&ctx->archive, sec->offset);
    ctx->stack[arg3 + 0] = values[arg2 + 0];
    ctx->stack[arg3 + 1] = values[arg2 + 1];
    ctx->stack[arg3 + 2] = values[arg2 + 2];
    ctx->stack[arg3 + 3] = values[arg2 + 3];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_dloadirs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_data_section* sec = validated_get_data_section(&ctx->archive, arg1);
    const lmnt_value* values = validated_get_data_block(&ctx->archive, sec->offset);
    size_t arg2v = value_to_size_t(ctx->stack[arg2]);
    if (arg2v < sec->count) {
        ctx->stack[arg3] = values[arg2v];
        return LMNT_OK;
    } else {
        return LMNT_ERROR_ACCESS_VIOLATION;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_dloadirv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_data_section* sec = validated_get_data_section(&ctx->archive, arg1);
    const lmnt_value* values = validated_get_data_block(&ctx->archive, sec->offset);
    size_t arg2v = value_to_size_t(ctx->stack[arg2]);
    if (sec->count >= 4 && arg2v <= sec->count - 4) {
        ctx->stack[arg3 + 0] = values[arg2v + 0];
        ctx->stack[arg3 + 1] = values[arg2v + 1];
        ctx->stack[arg3 + 2] = values[arg2v + 2];
        ctx->stack[arg3 + 3] = values[arg2v + 3];
        return LMNT_OK;
    } else {
        return LMNT_ERROR_ACCESS_VIOLATION;
    }
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_dseclen(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    const lmnt_data_section* sec = validated_get_data_section(&ctx->archive, arg1);
    ctx->stack[arg3] = (lmnt_value)(sec->count);
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_indexris(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (LMNT_UNLIKELY(isnan(ctx->stack[arg1]) || isinf(ctx->stack[arg1]))) return LMNT_ERROR_ACCESS_VIOLATION;
    size_t arg1v = value_to_size_t(ctx->stack[arg1]);
    if (arg1v + arg2 >= ctx->cur_stack_count) return LMNT_ERROR_ACCESS_VIOLATION;
    ctx->stack[arg3] = ctx->stack[arg1v + arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_indexrir(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    if (LMNT_UNLIKELY(isnan(ctx->stack[arg1]) || isinf(ctx->stack[arg1]))) return LMNT_ERROR_ACCESS_VIOLATION;
    if (LMNT_UNLIKELY(isnan(ctx->stack[arg3]) || isinf(ctx->stack[arg3]))) return LMNT_ERROR_ACCESS_VIOLATION;
    size_t arg1v = value_to_size_t(ctx->stack[arg1]);
    size_t arg3v = value_to_size_t(ctx->stack[arg3]);
    if (arg1v + arg2 >= ctx->cur_stack_count || arg3v >= ctx->cur_stack_count) return LMNT_ERROR_ACCESS_VIOLATION;
    ctx->stack[arg3v] = ctx->stack[arg1v + arg2];
    return LMNT_OK;
}

LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_interrupt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3)
{
    return LMNT_INTERRUPTED;
}

#endif