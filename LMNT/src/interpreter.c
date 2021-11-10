#include "lmnt/interpreter.h"
#include "lmnt/config.h"
#include "lmnt/validation.h"
#include "helpers.h"

#include <assert.h>
#include <math.h>
#include <string.h>


// We include exactly one dispatch_* header to execute functions
// Computed GOTOs are faster but only available where GNU C is
// Each header defines these functions with the same signatures:
LMNT_ATTR_FAST static inline lmnt_result execute_function(lmnt_ictx* ctx, const lmnt_code* defcode, const lmnt_instruction* instructions);
LMNT_ATTR_FAST static inline lmnt_result interrupt_function(lmnt_ictx* ctx);
static const char* const dispatch_method(void);
// Include the header
#if defined(LMNT_USE_COMPUTED_GOTOS) && LMNT_USE_COMPUTED_GOTOS
#include "dispatch_computed_goto.h"
#else
#include "dispatch_jumptable.h"
#endif

#include LMNT_MEMORY_HEADER
#if defined(LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS)
    #include LMNT_PRINTF_HEADER
#endif

lmnt_result lmnt_init(lmnt_ictx* ctx, char* mem, size_t mem_size)
{
    // sanity checks
    // a few hot-path optimisations rely on LMNT_OK being zero
    LMNT_STATIC_ASSERT(LMNT_OK == 0, "LMNT_OK must be equal to zero");

    memset(ctx, 0, sizeof(lmnt_ictx));

    ctx->memory_area = mem;
    ctx->memory_area_size = mem_size;

    // Determine how much space we want to use for frames/stack
    const size_t min_stack_count = 16;
    if (mem_size < min_stack_count * sizeof(lmnt_value))
        return LMNT_ERROR_MEMORY_SIZE;

    return LMNT_OK;
}

lmnt_result lmnt_load_archive(lmnt_ictx* ctx, const char* data, size_t data_size)
{
    LMNT_OK_OR_RETURN(lmnt_load_archive_begin(ctx));
    LMNT_OK_OR_RETURN(lmnt_load_archive_append(ctx, data, data_size));
    LMNT_OK_OR_RETURN(lmnt_load_archive_end(ctx));
    return LMNT_OK;
}

lmnt_result lmnt_load_archive_begin(lmnt_ictx* ctx)
{
    return lmnt_archive_init(&ctx->archive, ctx->memory_area, 0);
}

lmnt_result lmnt_load_archive_append(lmnt_ictx* ctx, const char* data, size_t data_size)
{
    if (data_size >= ctx->memory_area_size - ctx->archive.size)
        return LMNT_ERROR_MEMORY_SIZE;
    LMNT_MEMCPY(ctx->memory_area + ctx->archive.size, data, data_size);
    ctx->archive.size += data_size;
    return LMNT_OK;
}

lmnt_result lmnt_load_archive_end(lmnt_ictx* ctx)
{
    return LMNT_OK;
}

lmnt_result lmnt_load_inplace_archive(lmnt_ictx* ctx, const char* data, size_t data_size)
{
    // Don't copy the archive in, just use it where it is
    LMNT_OK_OR_RETURN(lmnt_archive_init(&ctx->archive, data, data_size));
    ctx->archive.flags |= LMNT_ARCHIVE_INPLACE;
    return LMNT_OK;
}

lmnt_result lmnt_prepare_archive(lmnt_ictx* ctx, lmnt_validation_result* vresult)
{
    lmnt_validation_result vr = lmnt_archive_validate(&ctx->archive, ctx->memory_area_size, &ctx->stack_count);
    if (vresult)
        *vresult = vr;
    if (vr != LMNT_VALIDATION_OK)
        return LMNT_ERROR_INVALID_ARCHIVE;

    const size_t constants_count = validated_get_constants_count(&ctx->archive);
    if (!(ctx->archive.flags & LMNT_ARCHIVE_INPLACE))
    {
        // Archive is loaded into our memory space
        // Constants are therefore already in the right place, and the stack starts with them
        ctx->stack = (lmnt_value*)validated_get_constants(&ctx->archive, 0);
    }
    else
    {
        // Archive is loaded in-place, so the stack starts at zero and we must copy the constants over
        ctx->stack = (lmnt_value*)(ctx->memory_area);
        const lmnt_value* constants = validated_get_constants(&ctx->archive, 0);
        LMNT_MEMCPY(ctx->stack, constants, constants_count * sizeof(lmnt_value));
    }
    ctx->writable_stack = ctx->stack + constants_count;

    // fill in extern defs' code value to point to the right extcall
    LMNT_OK_OR_RETURN(lmnt_archive_update_def_extcalls(&ctx->archive, ctx->extcalls, ctx->extcalls_count));

    return LMNT_OK;
}

lmnt_result lmnt_get_default_args(lmnt_ictx* ctx, const lmnt_def* def, const lmnt_value** args, lmnt_loffset* count)
{
    assert(ctx && def);
    LMNT_ENSURE_VALIDATED(&ctx->archive);
    if (!(def->flags & LMNT_DEFFLAG_HAS_DEFAULT_ARGS))
        return LMNT_ERROR_NOT_FOUND;
    const lmnt_data_section* section = validated_get_data_section(&ctx->archive, def->default_args_index);
    *args = validated_get_data_block(&ctx->archive, section->offset);
    *count = (section->count <= def->args_count) ? section->count : def->args_count; // don't report more args than the def actually has
    return LMNT_OK;
}

#if defined(LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS)
static inline void print_execution_context(lmnt_ictx* ctx, lmnt_loffset inst_idx, const lmnt_instruction op)
{
    LMNT_PRINTF("Eval[%02X]: % 12s %04X %04X %04X [", inst_idx, lmnt_get_opcode_info(op.opcode)->name, op.arg1, op.arg2, op.arg3);
    const size_t count = validated_get_constants_count(&ctx->archive) + ctx->cur_def->stack_count;
    for (size_t i = 0; i < count; ++i)
    {
        if (i) printf(", ");
        LMNT_PRINTF("%8.3f", ctx->stack[i]);
    }
    LMNT_PRINTF("] [%s %s %s %s]\n",
        (ctx->status_flags & LMNT_ISTATUS_CMP_EQ) ? "EQ" : "  ",
        (ctx->status_flags & LMNT_ISTATUS_CMP_LT) ? "LT" : "  ",
        (ctx->status_flags & LMNT_ISTATUS_CMP_GT) ? "GT" : "  ",
        (ctx->status_flags & LMNT_ISTATUS_CMP_UN) ? "UN" : "  ");
}
#endif


// This function assumes:
// - ctx->cur_def has been set to the def to be executed
// - ctx->cur_instr is set to 0 for new execution or its previous value for resume
// - ctx->writable_stack has been pre-prepared with args if this is new execution
LMNT_ATTR_FAST static lmnt_result execute(lmnt_ictx* ctx, lmnt_value* rvals, const lmnt_offset rvals_count)
{
    assert(ctx && ctx->archive.data);
    assert(ctx && ctx->stack && ctx->stack_count);
    assert(ctx && ctx->cur_def);

    const lmnt_def* const def = ctx->cur_def;

    if (LMNT_UNLIKELY(rvals && rvals_count < def->rvals_count))
        return LMNT_ERROR_RVALS_MISMATCH;

    lmnt_result opresult = LMNT_OK;
    if (LMNT_LIKELY(!(def->flags & LMNT_DEFFLAG_EXTERN)))
    {
        // Get code
        const lmnt_code* defcode = validated_get_code(&ctx->archive, def->code);
        const lmnt_instruction* instructions = validated_get_code_instructions(&ctx->archive, def->code);

        // Execute using function defined in dispatch_*.h
        opresult = execute_function(ctx, defcode, instructions);
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
    // If we hit a return instruction or a branch past the function end, that's an OK return
    if (opresult == LMNT_RETURNING || opresult == LMNT_BRANCHING) {
        opresult = LMNT_OK;
    }

    // If OK and we have a buffer to write to, copy out return values and return the count populated
    if (LMNT_LIKELY(opresult == LMNT_OK && rvals))
    {
        LMNT_MEMCPY(rvals, ctx->writable_stack + def->args_count, def->rvals_count * sizeof(lmnt_value));
        return def->rvals_count;
    }
    // Otherwise, just return OK or a failure code
    return opresult;
}

LMNT_ATTR_FAST lmnt_result lmnt_execute(
    lmnt_ictx* ctx, const lmnt_def* def,
    lmnt_value* rvals, const lmnt_offset rvals_count)
{
    assert(ctx && ctx->stack && ctx->stack_count);
    assert(def);
    // Set our current instruction to be the start of the requested def
    ctx->cur_def = def;
    ctx->cur_instr = 0;
    ctx->cur_stack_count = (size_t)validated_get_constants_count(&ctx->archive) + (size_t)def->stack_count;
    // Run main execute loop
    return execute(ctx, rvals, rvals_count);
}

LMNT_ATTR_FAST lmnt_result lmnt_resume(
    lmnt_ictx* ctx, const lmnt_def* def,
    lmnt_value* rvals, const lmnt_offset rvals_count)
{
    assert(ctx);
    assert(def);
    // Make sure the def we're resuming is the one the user thinks we are
    if (LMNT_UNLIKELY(ctx->cur_def != def))
        return LMNT_ERROR_DEF_MISMATCH;
    // Run main execute loop
    return execute(ctx, rvals, rvals_count);
}

lmnt_result lmnt_interrupt(lmnt_ictx* ctx)
{
    assert(ctx);
    return interrupt_function(ctx);
}

lmnt_result lmnt_update_args(
    lmnt_ictx* ctx, const lmnt_def* def,
    const lmnt_offset offset, const lmnt_value* args, const lmnt_offset count)
{
    assert(ctx);
    assert(ctx->stack && ctx->stack_count);
    assert(def);
    assert(args || count == 0);
    if (LMNT_UNLIKELY(offset + count > ctx->stack_count))
        return LMNT_ERROR_ACCESS_VIOLATION;
    if (LMNT_UNLIKELY(offset + count > def->args_count))
        return LMNT_ERROR_ARGS_MISMATCH;
    // Copy args into the writable stack
    LMNT_MEMCPY(ctx->writable_stack + offset, args, count * sizeof(lmnt_value));
    return LMNT_OK;
}

lmnt_result lmnt_find_def(const lmnt_ictx* ctx, const char* name, const lmnt_def** def)
{
    return lmnt_archive_find_def(&ctx->archive, name, def);
}

const char* lmnt_get_dispatch_method(void)
{
    return dispatch_method();
}