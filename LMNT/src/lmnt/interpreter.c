#include "lmnt/interpreter.h"
#include "lmnt/config.h"
#include "lmnt/validation.h"

#include <assert.h>
#include <math.h>

extern lmnt_op_fn lmnt_op_functions[LMNT_OP_END];
extern lmnt_op_fn lmnt_interrupt_functions[LMNT_OP_END];

static lmnt_result get_stack_start(const lmnt_ictx* ctx, lmnt_value** value)
{
    // Slightly naughty: we need to return a non-const pointer
    // to the stack start since everything after the constants read-write
    return lmnt_get_constants(&ctx->archive, 0, (const lmnt_value**)value);
}

lmnt_result lmnt_ictx_init(lmnt_ictx* ctx, char* mem, size_t mem_size)
{
    ctx->memory_area = mem;
    ctx->memory_area_size = mem_size;

    // Determine how much space we want to use for frames/stack
    const size_t min_stack_count = 256;
    if (mem_size < min_stack_count * sizeof(lmnt_value))
        return LMNT_ERROR_MEMORY_SIZE;

    ctx->archive.data = NULL;
    ctx->archive.size = 0;
    return LMNT_OK;
}

lmnt_result lmnt_ictx_load_archive(lmnt_ictx* ctx, const char* data, size_t data_size)
{
    LMNT_OK_OR_RETURN(lmnt_ictx_load_archive_begin(ctx));
    LMNT_OK_OR_RETURN(lmnt_ictx_load_archive_append(ctx, data, data_size));
    LMNT_OK_OR_RETURN(lmnt_ictx_load_archive_end(ctx));
    return LMNT_OK;
}

lmnt_result lmnt_ictx_load_archive_begin(lmnt_ictx* ctx)
{
    return lmnt_archive_init(&ctx->archive, ctx->memory_area, 0);
}

lmnt_result lmnt_ictx_load_archive_append(lmnt_ictx* ctx, const char* data, size_t data_size)
{
    if (data_size >= ctx->memory_area_size - ctx->archive.size)
        return LMNT_ERROR_MEMORY_SIZE;
    LMNT_MEMCPY(ctx->memory_area + ctx->archive.size, data, data_size);
    ctx->archive.size += data_size;
    return LMNT_OK;
}

lmnt_result lmnt_ictx_load_archive_end(lmnt_ictx* ctx)
{
    LMNT_OK_OR_RETURN(get_stack_start(ctx, &ctx->stack));
    ctx->writable_stack = (lmnt_value*)(ctx->memory_area + ctx->archive.size);
    ctx->stack_count = ((ctx->memory_area + ctx->memory_area_size) - (const char*)(ctx->stack)) / sizeof(lmnt_value);
    return LMNT_OK;
}

lmnt_result lmnt_ictx_prepare_archive(lmnt_ictx* ctx, lmnt_validation_result* vresult)
{
    lmnt_offset constants_count;
    LMNT_OK_OR_RETURN(lmnt_get_constants_count(&ctx->archive, &constants_count));
    const lmnt_offset writable_stack_count = (lmnt_offset)(ctx->stack_count - constants_count);
    lmnt_validation_result vr = lmnt_archive_validate(&ctx->archive, constants_count, writable_stack_count);
    if (vresult)
        *vresult = vr;
    if (vr != LMNT_VALIDATION_OK)
        return LMNT_ERROR_INVALID_ARCHIVE;

    // fill in extern defs' code value to point to the right extcall
    lmnt_result er = lmnt_update_def_extcalls(&ctx->archive, ctx->extcalls, ctx->extcalls_count);
    if (er != LMNT_OK)
        return LMNT_ERROR_MISSING_EXTCALL;

    return LMNT_OK;
}

LMNT_ATTR_FAST static inline lmnt_result execute_instruction(lmnt_ictx* ctx, const lmnt_instruction op)
{
    assert(op.opcode < LMNT_OP_END);
    assert(ctx->op_functions[op.opcode]);
    return ctx->op_functions[op.opcode](ctx, op.arg1, op.arg2, op.arg3);
}

// This function assumes:
// - ctx->cur_def has been set to the def to be executed
// - ctx->cur_instr is set to 0 for new execution or its previous value for resume
// - ctx->writable_stack has been pre-prepared with args if this is new execution
LMNT_ATTR_FAST static lmnt_result execute(lmnt_ictx* ctx, lmnt_value* rvals, const lmnt_offset rvals_count)
{
    assert(ctx && ctx->archive.data);
    assert(ctx->stack && ctx->stack_count);
    assert(ctx->cur_def);

    const lmnt_def* const def = ctx->cur_def;

    if (LMNT_UNLIKELY(rvals && rvals_count < def->rvals_count))
        return LMNT_ERROR_RVALS_MISMATCH;

    // Work out if we're executing a locally-defined block or an extern one
    const lmnt_code* defcode;
    const lmnt_instruction* instructions;
    if ((def->flags & LMNT_DEFFLAG_EXTERN) == 0)
    {
        // Get code
        LMNT_OK_OR_RETURN(lmnt_get_code(&ctx->archive, def->code, &defcode));
        LMNT_OK_OR_RETURN(lmnt_get_code_instructions(&ctx->archive, def->code, &instructions));
    }

    // Make sure the context is set up to use the real ops
    ctx->op_functions = lmnt_op_functions;

    lmnt_result opresult = LMNT_OK;
    if ((def->flags & LMNT_DEFFLAG_EXTERN) == 0)
    {
        // Grab the current instruction as a local rather than updating the ctx every time - faster
        lmnt_loffset instr;
        const lmnt_loffset icount = defcode->instructions_count;
        for (instr = ctx->cur_instr; instr < icount; ++instr)
        {
            opresult = execute_instruction(ctx, instructions[instr]);
            if (LMNT_UNLIKELY(opresult != LMNT_OK)) break;
        }
        ctx->cur_instr = instr;
    }
    else
    {
        const lmnt_extcall_info* extcall;
        LMNT_OK_OR_RETURN(lmnt_ictx_extcall_get(ctx, def->code, &extcall));

        lmnt_value* const eargs = &ctx->writable_stack[0];
        lmnt_value* const ervals = &ctx->writable_stack[extcall->args_count];
        opresult = extcall->function(ctx, eargs, extcall->args_count, ervals, extcall->rvals_count);
    }

    // If we finished or hit an error, clear the context's current def
    if (LMNT_LIKELY(opresult != LMNT_INTERRUPTED))
        ctx->cur_def = NULL;

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

LMNT_ATTR_FAST lmnt_result lmnt_execute(
    lmnt_ictx* ctx, const lmnt_def* def,
    lmnt_value* rvals, const lmnt_offset rvals_count)
{
    assert(ctx);
    assert(ctx->stack && ctx->stack_count);
    assert(def);
    // Set our current instruction to be the start of the requested def
    ctx->cur_def = def;
    ctx->cur_instr = 0;
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
    ctx->op_functions = lmnt_interrupt_functions;
    return LMNT_OK;
}

lmnt_result lmnt_update_args(
    lmnt_ictx* ctx, const lmnt_def* def,
    const lmnt_value* args, const lmnt_offset count, const lmnt_offset offset)
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

lmnt_result lmnt_update_arg(
    lmnt_ictx* ctx, const lmnt_def* def,
    const lmnt_value arg, const lmnt_offset offset)
{
    assert(ctx);
    assert(ctx->stack && ctx->stack_count);
    assert(def);
    if (LMNT_UNLIKELY(offset >= ctx->stack_count))
        return LMNT_ERROR_ACCESS_VIOLATION;
    if (LMNT_UNLIKELY(offset >= def->args_count))
        return LMNT_ERROR_ARGS_MISMATCH;
    // Copy arg into the writable stack
    ctx->writable_stack[offset] = arg;
    return LMNT_OK;
}