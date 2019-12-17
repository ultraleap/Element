#ifndef LMNT_JIT_REGHELPERS_X86_H
#define LMNT_JIT_REGHELPERS_X86_H

#include "jithelpers.h"

#define FPREG_MAX 16

// Define FP register characteristics
#define LMNT_FPREG_V_START 2        // leave XMM0-1 free for extern args/rvals
#if _WIN32
#define LMNT_FPREG_NV_START  6  // XMM0-5 are volatile on Windows
#else
#define LMNT_FPREG_NV_START 16  // all XMMs are volatile on non-Windows
#endif
#define LMNT_FPREG_V_END LMNT_FPREG_NV_START
#define LMNT_FPREG_NV_END 16

#define LMNT_FPREG_TOTAL_COUNT LMNT_FPREG_NV_END

#define LMNT_FPREG_START LMNT_FPREG_V_START
#if defined(LMNT_JIT_X86_64_ALLOW_NV_REGISTERS)
#define LMNT_FPREG_END LMNT_FPREG_TOTAL_COUNT
#else
#define LMNT_FPREG_END LMNT_FPREG_NV_START
#endif

#define LMNT_FPREG_PREFIX "xmm"


typedef struct
{
    lmnt_offset stackpos;
    size_t count;
    reg_status_flags flags;
    int importance;
} reg_status;

typedef struct
{
    size_t start;
    size_t end;
} fpreg_set;

struct jit_fpreg_data_s
{
    size_t start;
    size_t end;
    fpreg_set preferred;
    fpreg_set fallback;
    reg_status xmm[FPREG_MAX];
};


static int chooseExistingRegister(jit_compile_state* state, size_t* reg, size_t scount, int importance)
{
    int lowprio = INT_MAX;
    size_t lowreg = 0;
    for (size_t i = state->fpreg->preferred.start; i < state->fpreg->preferred.end; ++i) {
        if (state->fpreg->xmm[i].count == 0) {
            // It's free real estate
            *reg = i;
            return 1;
        }
    }
    for (size_t i = state->fpreg->fallback.start; i < state->fpreg->fallback.end; ++i) {
        if (state->fpreg->xmm[i].count == 0) {
            // It's free real estate in a more volatile part of town
            *reg = i;
            return 1;
        }
    }
    for (size_t i = state->fpreg->start; i < state->fpreg->end; ++i) {
        if (state->fpreg->xmm[i].importance < lowprio) {
            lowprio = state->fpreg->xmm[i].importance;
            lowreg = i;
        }
    }
    if (importance > lowprio) {
        *reg = lowreg;
        return 1;
    }
    else {
        return 0;
    }
}

static int chooseExistingScalarRegister(jit_compile_state* state, size_t* reg, int importance)
{
    return chooseExistingRegister(state, reg, 1, importance);
}

static int chooseExistingVectorRegister(jit_compile_state* state, size_t* reg, int importance)
{
    return chooseExistingRegister(state, reg, 4, importance);
}


static inline void updateRegister(jit_compile_state* state, size_t reg, lmnt_offset spos, size_t scount, int importance, access_type modified)
{
    assert(state->fpreg->xmm[reg].count == 0);
    state->fpreg->xmm[reg].stackpos = spos;
    state->fpreg->xmm[reg].count = scount;
    updateRegisterState(state, reg, importance, modified);
}

static inline void updateRegisterState(jit_compile_state* state, size_t reg, int importance, access_type modified)
{
    state->fpreg->xmm[reg].importance = importance;
    state->fpreg->xmm[reg].flags |= ((modified & ACCESSTYPE_WRITE) ? REGST_MODIFIED : REGST_NONE);
}

static inline size_t getRegisterCount(jit_compile_state* state, size_t reg)
{
    return state->fpreg->xmm[reg].count;
}

static inline int isRegisterInitialised(jit_compile_state* state, size_t reg)
{
    return (state->fpreg->xmm[reg].flags & REGST_INITIALISED);
}

static inline int allowIndividualLaneAccess(jit_compile_state* state)
{
    return 0;
}

static void initialiseRegister(jit_compile_state* state, size_t reg)
{
    const lmnt_offset spos = state->fpreg->xmm[reg].stackpos;
    const size_t sz = state->fpreg->xmm[reg].count;
    assert(sz == 4 || sz == 1);

    if (sz == 4)
        platformReadVectorToRegister(state, reg, spos);
    else if (sz == 1)
        platformReadScalarToRegister(state, reg, spos);
    state->fpreg->xmm[reg].flags |= REGST_INITIALISED;
}

static int flushRegister(jit_compile_state* state, size_t reg)
{
    const lmnt_offset spos = state->fpreg->xmm[reg].stackpos;
    const size_t sz = state->fpreg->xmm[reg].count;
    assert(sz == 4 || sz == 1);

    // If not modified, we have nothing to do here
    if (state->fpreg->xmm[reg].flags & REGST_MODIFIED)
    {
        if (sz == 4)
            platformWriteVectorFromRegister(state, spos, reg);
        else if (sz == 1)
            platformWriteScalarFromRegister(state, spos, reg);
        state->fpreg->xmm[reg].flags &= (~REGST_MODIFIED);
        return 1;
    }
    return 0;
}

static inline void notifyRegisterWritten(jit_compile_state* state, size_t reg, size_t writeSize)
{
    state->fpreg->xmm[reg].flags |= (REGST_INITIALISED | REGST_MODIFIED);
}

static void evictRegister(jit_compile_state* state, size_t reg)
{
    reg_status* const s = &state->fpreg->xmm[reg];
    JIT_DEBUG_PRINTF(
        "evictRegister: evicting %s%zu, size %zu, was mapped to %d (0x%02X)\n",
        LMNT_FPREG_PREFIX, reg, s->count, s->stackpos, s->stackpos);
    JIT_STATS_PERFORM(++state->stats.reg_evicted);
    s->count = 0;
    s->flags = REGST_NONE;
}

static void writeAndEvictRegisters(jit_compile_state* state, size_t reg, size_t scount)
{
    writeAndEvictRegister(state, reg);
}

static void writeAndEvictRegister(jit_compile_state* state, size_t reg)
{
    reg_status* const s = &state->fpreg->xmm[reg];
    const size_t sz = s->count;
    if (sz == 0) return;
    assert(sz == 1 || sz == 4);

    if ((s->flags & (REGST_INITIALISED | REGST_MODIFIED)) == (REGST_INITIALISED | REGST_MODIFIED))
    {
        JIT_DEBUG_PRINTF(
            "writeAndEvictRegister: writing modified %s%zu to %d (0x%02X) before eviction, size %zu\n",
            LMNT_FPREG_PREFIX, reg, s->stackpos, s->stackpos, sz);
        if (sz == 4)
            platformWriteVectorFromRegister(state, s->stackpos, reg);
        else if (sz == 1)
            platformWriteScalarFromRegister(state, s->stackpos, reg);
        JIT_STATS_PERFORM(++state->stats.reg_evicted_written);
    }

    evictRegister(state, reg);
}


static overlap_type checkForOverlap(jit_compile_state* state, lmnt_offset spos, size_t scount, size_t* reg, int* importance)
{
    for (size_t i = state->fpreg->start; i < state->fpreg->end; ++i)
    {
        const reg_status s = state->fpreg->xmm[i];
        if (s.count > 0 && ((s.stackpos >= spos && s.stackpos < spos + scount) || (spos >= s.stackpos && spos < s.stackpos + s.count)))
        {
            *reg = i;
            *importance = s.importance;
            // overlap
            if (s.stackpos == spos) {
                if (scount == s.count)
                    return OVERLAP_ALIGNED;
                else if (scount < s.count)
                    return OVERLAP_ALIGNED_SUBSET;
                else
                    return OVERLAP_ALIGNED_SUPERSET;
            }
            else if (spos > s.stackpos && spos + scount <= s.stackpos + s.count) {
                return OVERLAP_SUBSET;
            }
            else if (spos < s.stackpos && spos + scount >= s.stackpos + s.count) {
                return OVERLAP_SUPERSET;
            }
            else {
                return OVERLAP_UNALIGNED;
            }
        }
    }
    return OVERLAP_NONE;
}

#if defined(LMNT_JIT_DEBUG_VALIDATE_REGCACHE)
static lmnt_result validateRegCache(jit_compile_state* state)
{
    for (size_t i = state->fpreg->start; i < state->fpreg->end; ++i)
    {
        const reg_status s1 = state->fpreg->xmm[i];
        if (s1.count == 0) {
            if (s1.flags != REGST_NONE) {
                JIT_DEBUG_PRINTF(
                    "validateRegCache: ERROR - %s%zu count = 0 but flags = %d, stackpos = %u\n",
                    LMNT_FPREG_PREFIX, i, s1.flags, s1.stackpos);
                return LMNT_ERROR_INTERNAL;
            }
            continue;
        }
        for (size_t j = state->fpreg->start; j < state->fpreg->end; ++j)
        {
            if (i == j) continue;
            const reg_status s2 = state->fpreg->xmm[j];
            if (s2.count == 0) continue;

            if (s1.stackpos >= s2.stackpos && s1.stackpos < s2.stackpos + s2.count) {
                JIT_DEBUG_PRINTF(
                    "validateRegCache: ERROR - %s%zu (0x%02X+%zu) overlap with %s%zu (0x%02X+%zu)\n",
                    LMNT_FPREG_PREFIX, j, s2.stackpos, s2.count,
                    LMNT_FPREG_PREFIX, i, s1.stackpos, s1.count);
                return LMNT_ERROR_INTERNAL;
            }
            if (s2.stackpos >= s1.stackpos && s2.stackpos < s1.stackpos + s1.count) {
                JIT_DEBUG_PRINTF(
                    "validateRegCache: ERROR - %s%zu (0x%02X+%zu) overlap with %s%zu (0x%02X+%zu)\n",
                    LMNT_FPREG_PREFIX, i, s1.stackpos, s1.count,
                    LMNT_FPREG_PREFIX, j, s2.stackpos, s2.count);
                return LMNT_ERROR_INTERNAL;
            }
        }
    }

    return LMNT_OK;
}
#endif


#endif