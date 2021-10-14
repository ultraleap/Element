#include "lmnt/common.h"
#include "lmnt/interpreter.h"
#include "lmnt/jit.h"
#include "lmnt/platform.h"
#include "jit/hosthelpers.h"
#include "helpers.h"

#include LMNT_MEMORY_HEADER

#if defined(LMNT_JIT_HAS_X86_64)
lmnt_result lmnt_jit_x86_64_compile(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_fn_data* fndata, lmnt_jit_compile_stats* stats);
#endif
#if defined(LMNT_JIT_HAS_ARMV7A)
lmnt_result lmnt_jit_armv7a_compile(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_fn_data* fndata, lmnt_jit_compile_stats* stats);
#endif
#if defined(LMNT_JIT_HAS_ARMV7M)
lmnt_result lmnt_jit_armv7m_compile(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_fn_data* fndata, lmnt_jit_compile_stats* stats);
#endif
#if defined(LMNT_JIT_HAS_ARM64)
lmnt_result lmnt_jit_arm64_compile(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_fn_data* fndata, lmnt_jit_compile_stats* stats);
#endif


LMNT_ATTR_FAST lmnt_result lmnt_jit_execute(
    lmnt_ictx* ctx, const lmnt_def* def, const lmnt_jit_fn_data* fndata,
    lmnt_value* rvals, const lmnt_offset rvals_count)
{
    assert(ctx && ctx->archive.data);
    assert(ctx->stack && ctx->stack_count);

    ctx->cur_def = def;
    ctx->cur_instr = (lmnt_loffset)-1;
    lmnt_offset consts_count = validated_get_constants_count(&ctx->archive);
    ctx->cur_stack_count = (size_t)consts_count + def->stack_count;

    if (LMNT_UNLIKELY(rvals && rvals_count < def->rvals_count))
        return LMNT_ERROR_RVALS_MISMATCH;

    lmnt_result opresult = LMNT_OK;
    if ((def->flags & LMNT_DEFFLAG_EXTERN) == 0)
    {
        opresult = fndata->function(ctx);
    }
    else
    {
        const lmnt_extcall_info* extcall;
        LMNT_OK_OR_RETURN(lmnt_extcall_get(ctx, def->code, &extcall));

        lmnt_value* const eargs = &ctx->writable_stack[0];
        lmnt_value* const ervals = &ctx->writable_stack[extcall->args_count];
        opresult = extcall->function(ctx, extcall, eargs, ervals);
    }

    // If we finished or hit an error, clear the context's current def
    if (LMNT_LIKELY(opresult != LMNT_INTERRUPTED)) {
        ctx->cur_def = NULL;
        ctx->cur_stack_count = 0;
    }

    // If OK and we have a buffer to write to, copy out return values and return the count populated
    if (LMNT_LIKELY(opresult == LMNT_OK && rvals))
    {
        // TODO: make this more robust
        LMNT_MEMCPY(rvals, &ctx->writable_stack[def->args_count], def->rvals_count * sizeof(lmnt_value));
        return def->rvals_count;
    }
    // Otherwise, just return OK or a failure code
    return opresult;
}


lmnt_result lmnt_jit_compile(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_target target, lmnt_jit_fn_data* fndata)
{
    if (target == LMNT_JIT_TARGET_CURRENT)
        target = LMNT_JIT_TARGET_NATIVE;
    switch (target)
    {
#if defined(LMNT_JIT_HAS_X86_64)
    case LMNT_JIT_TARGET_X86_64: return lmnt_jit_x86_64_compile(ctx, def, fndata, NULL);
#endif
#if defined(LMNT_JIT_HAS_ARMV7A)
    case LMNT_JIT_TARGET_ARMV7A: return lmnt_jit_armv7a_compile(ctx, def, fndata, NULL);
#endif
#if defined(LMNT_JIT_HAS_ARMV7M)
    case LMNT_JIT_TARGET_ARMV7M: return lmnt_jit_armv7m_compile(ctx, def, fndata, NULL);
#endif
#if defined(LMNT_JIT_HAS_ARM64)
    case LMNT_JIT_TARGET_ARM64:  return lmnt_jit_arm64_compile(ctx, def, fndata, NULL);
#endif
    default: return LMNT_ERROR_NO_IMPL;
    }
}

#if defined(LMNT_JIT_COLLECT_STATS)
lmnt_result lmnt_jit_compile_with_stats(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_target target, lmnt_jit_fn_data* fndata, lmnt_jit_compile_stats* stats)
{
    if (target == LMNT_JIT_TARGET_CURRENT)
        target = LMNT_JIT_TARGET_NATIVE;
    switch (target)
    {
#if defined(LMNT_JIT_HAS_X86_64)
    case LMNT_JIT_TARGET_X86_64: return lmnt_jit_x86_64_compile(ctx, def, fndata, stats);
#endif
#if defined(LMNT_JIT_HAS_ARMV7A)
    case LMNT_JIT_TARGET_ARMV7A: return lmnt_jit_armv7a_compile(ctx, def, fndata, stats);
#endif
#if defined(LMNT_JIT_HAS_ARMV7M)
    case LMNT_JIT_TARGET_ARMV7M: return lmnt_jit_armv7m_compile(ctx, def, fndata, stats);
#endif
#if defined(LMNT_JIT_HAS_ARM64)
    case LMNT_JIT_TARGET_ARM64:  return lmnt_jit_arm64_compile(ctx, def, fndata, stats);
#endif
    default: return LMNT_ERROR_NO_IMPL;
    }
}
#endif

lmnt_result lmnt_jit_delete_function(lmnt_jit_fn_data* fndata)
{
    LMNT_JIT_FREE_CFN_MEMORY(fndata->buffer, fndata->codesize);
    return LMNT_OK;
}
