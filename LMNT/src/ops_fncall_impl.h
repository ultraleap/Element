#ifndef LMNT_OPS_FNCALL_IMPL_H
#define LMNT_OPS_FNCALL_IMPL_H

#include <string.h>
#include <assert.h>
#include <limits.h>
#include "lmnt/common.h"
#include "lmnt/interpreter.h"
#include "helpers.h"

#if !defined(LMNT_INLINE_OP)
#define LMNT_INLINE_OP
#endif


LMNT_ATTR_FAST static LMNT_INLINE_OP inline lmnt_result lmnt_op_extcall(lmnt_ictx* ctx, lmnt_offset deflo, lmnt_offset defhi, lmnt_offset stack_pos)
{
    const lmnt_loffset def_offset = LMNT_COMBINE_OFFSET(deflo, defhi);
    // Get code
    const lmnt_def* def = validated_get_def(&ctx->archive, def_offset);

    // During the archive prep stage, the extcall index is put into the def's code member
    const lmnt_extcall_info* extcall;
    LMNT_OK_OR_RETURN(lmnt_extcall_get(ctx, def->code, &extcall));

    lmnt_value* const eargs = &ctx->stack[stack_pos];
    lmnt_value* const ervals = &ctx->stack[stack_pos + extcall->args_count];
    return extcall->function(ctx, extcall, eargs, ervals);
}

#endif