#ifndef LMNT_JIT_H
#define LMNT_JIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "lmnt/config.h"
#include "lmnt/jitconfig.h"

typedef lmnt_result(*lmnt_jit_fn)(lmnt_ictx* ctx);

typedef struct
{
    const lmnt_def* def;
    lmnt_jit_fn function;
    void* buffer;
    size_t codesize;
    void* interrupt;
    void* interruptible_start;
    void* interruptible_end;
} lmnt_jit_fn_data;

typedef struct
{
    size_t codesize;
    size_t reg_alloc;
    size_t reg_aligned;
    size_t reg_unaligned;
    size_t reg_evicted;
    size_t reg_evicted_written;
} lmnt_jit_compile_stats;

typedef enum
{
    LMNT_JIT_TARGET_CURRENT = 0,
    LMNT_JIT_TARGET_X86_64 = 1,
    LMNT_JIT_TARGET_ARMV7A = 2,
    LMNT_JIT_TARGET_ARMV7M = 3,
} lmnt_jit_target;

lmnt_result lmnt_jit_compile(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_target target, lmnt_jit_fn_data* fndata);
#if defined(LMNT_JIT_COLLECT_STATS)
lmnt_result lmnt_jit_compile_with_stats(lmnt_ictx* ctx, const lmnt_def* def, lmnt_jit_target target, lmnt_jit_fn_data* fn, lmnt_jit_compile_stats* stats);
#endif

lmnt_result lmnt_jit_delete_function(lmnt_jit_fn_data* fndata);

LMNT_ATTR_FAST lmnt_result lmnt_jit_execute(
    lmnt_ictx* ctx, const lmnt_jit_fn_data* fndata,
    lmnt_value* rvals, const lmnt_offset rvals_count);

bool lmnt_jit_is_interruptible(const lmnt_jit_fn_data* fndata, void* inst_pointer);

#ifdef __cplusplus
}
#endif

#endif
