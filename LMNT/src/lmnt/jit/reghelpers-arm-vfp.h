#ifndef LMNT_JIT_REGHELPERS_ARM_H
#define LMNT_JIT_REGHELPERS_ARM_H

#include "jithelpers.h"

#define FPREG_MAX 32
#define FPREG_BANK_SIZE 4

// Define FP register characteristics
#define LMNT_FPREG_V_START 8        // leave s0-7 free for extern args/rvals
#define LMNT_FPREG_NV_START 16
#define LMNT_FPREG_V_END LMNT_FPREG_NV_START
#define LMNT_FPREG_NV_END 32

#define LMNT_FPREG_TOTAL_COUNT LMNT_FPREG_NV_END

#define LMNT_FPREG_START LMNT_FPREG_V_START
#if defined(LMNT_JIT_ARM_ALLOW_NV_REGISTERS)
#define LMNT_FPREG_END LMNT_FPREG_TOTAL_COUNT
#else
#define LMNT_FPREG_END LMNT_FPREG_NV_START
#endif

#define LMNT_FPREG_PREFIX "s"

static inline size_t nextbank(size_t r)
{
    return (r % FPREG_BANK_SIZE == 0) ? r : r + (FPREG_BANK_SIZE - (r%FPREG_BANK_SIZE));
}

static inline size_t thisbank(size_t r)
{
    return r - (r % FPREG_BANK_SIZE);
}

typedef struct
{
    size_t count;
    size_t index;
    lmnt_offset stackpos;
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
    reg_status s[FPREG_MAX];
};


#define S(i) state->fpreg->s[i]

static bool chooseExistingScalarRegister(jit_compile_state* state, size_t* reg, int importance)
{
    int lowprio = INT_MAX;
    size_t lowreg = 0;
    for (size_t i = state->fpreg->preferred.start; i < state->fpreg->preferred.end; ++i) {
        if (S(i).count == 0) {
            // It's free real estate
            *reg = i;
            return true;
        }
    }
    for (size_t i = state->fpreg->fallback.start; i < state->fpreg->fallback.end; ++i) {
        if (S(i).count == 0) {
            // It's free real estate in a more volatile part of town
            *reg = i;
            return true;
        }
    }

    // Try to clear out other scalar registers
    for (size_t i = state->fpreg->start; i < state->fpreg->end; ++i) {
        if (S(i).importance < lowprio && S(i).count == 1) {
            lowprio = S(i).importance;
            lowreg = i;
        }
    }
    if (importance > lowprio) {
        *reg = lowreg;
        return true;
    }

    // Try to clear out other vector registers
    size_t start = nextbank(state->fpreg->start);
    size_t end = thisbank(state->fpreg->start);
    for (size_t i = start; i < end; i += FPREG_BANK_SIZE) {
        if (S(i).importance < lowprio && S(i).count == FPREG_BANK_SIZE) {
            lowprio = S(i).importance;
            lowreg = i;
        }
    }
    if (importance > lowprio) {
        *reg = lowreg;
        return true;
    }

    return false;
}

static bool chooseExistingVectorRegister(jit_compile_state* state, size_t* reg, int importance)
{
    int lowprio = INT_MAX;
    size_t lowreg = 0;
    size_t pstart = nextbank(state->fpreg->preferred.start);
    size_t pend = thisbank(state->fpreg->preferred.end);
    for (size_t i = pstart; i < pend; i += FPREG_BANK_SIZE) {
        int count = 0;
        for (size_t j = 0; j < FPREG_BANK_SIZE; ++j)
            count += (S(i + j).count != 0);
        if (count == 0) {
            // It's free real estate
            *reg = i;
            return true;
        }
    }
    size_t fstart = nextbank(state->fpreg->fallback.start);
    size_t fend = thisbank(state->fpreg->fallback.end);
    for (size_t i = fstart; i < fend; i += FPREG_BANK_SIZE) {
        int count = 0;
        for (size_t j = 0; j < FPREG_BANK_SIZE; ++j)
            count += (S(i + j).count != 0);
        if (count == 0) {
            // It's free real estate in a more volatile part of town
            *reg = i;
            return true;
        }
    }

    // Try to clear out other vector registers
    size_t start = nextbank(state->fpreg->start);
    size_t end = thisbank(state->fpreg->end);
    for (size_t i = start; i < end; i += FPREG_BANK_SIZE) {
        if (S(i).importance < lowprio && S(i).count == FPREG_BANK_SIZE) {
            lowprio = S(i).importance;
            lowreg = i;
        }
    }
    if (importance > lowprio) {
        *reg = lowreg;
        return true;
    }

    // Try to clear out other scalar registers
    for (size_t i = start; i < end; i += FPREG_BANK_SIZE) {
        if (S(i).count == FPREG_BANK_SIZE) continue;
        int chighprio = INT_MIN;
        for (size_t j = 0; j < FPREG_BANK_SIZE; ++j)
            chighprio = (S(i).count != 0 && S(i+j).importance > chighprio) ? S(i+j).importance : chighprio;
        if (chighprio < lowprio) {
            lowprio = chighprio;
            lowreg = i;
        }
    }
    if (importance > lowprio) {
        *reg = lowreg;
        return true;
    }

    return false;
}


static inline void updateRegister(jit_compile_state* state, size_t reg, lmnt_offset spos, size_t scount, int importance, access_type modified)
{
    if (scount == FPREG_BANK_SIZE) {
        // Updating N at once
        size_t roff = (reg - thisbank(reg));
        for (size_t i = 0; i < FPREG_BANK_SIZE; ++i) {
            size_t creg = thisbank(reg) + i;
            assert(S(creg).count == 0);
            S(creg).stackpos = (spos - (lmnt_offset)roff) + (lmnt_offset)i;
            S(creg).count = scount;
            S(creg).index = i;
        }
    } else {
        assert(S(reg).count == 0);
        S(reg).stackpos = spos;
        S(reg).count = scount;
        S(reg).index = 0;
    }
    updateRegisterState(state, reg, importance, modified);
}

static inline void updateRegisterState(jit_compile_state* state, size_t reg, int importance, access_type modified)
{
    if (S(reg).count == FPREG_BANK_SIZE) {
        // Updating N at once
        for (size_t i = 0; i < FPREG_BANK_SIZE; ++i) {
            size_t creg = thisbank(reg) + i;
            S(creg).importance = importance;
            S(creg).flags |= ((modified & ACCESSTYPE_WRITE) ? REGST_MODIFIED : REGST_NONE);
        }
    } else {
        S(reg).importance = importance;
        S(reg).flags |= ((modified & ACCESSTYPE_WRITE) ? REGST_MODIFIED : REGST_NONE);
    }
}

static inline size_t getRegisterCount(jit_compile_state* state, size_t reg)
{
    return state->fpreg->s[reg].count;
}

static inline bool isRegisterInitialised(jit_compile_state* state, size_t reg)
{
    return (state->fpreg->s[reg].flags & REGST_INITIALISED);
}

static bool allowIndividualLaneAccess(jit_compile_state* state)
{
    return true;
}

static void initialiseRegister(jit_compile_state* state, size_t reg)
{
    size_t sz = S(reg).count;
    assert(sz == FPREG_BANK_SIZE || sz == 1);
    if (sz == FPREG_BANK_SIZE) {
        const size_t breg = thisbank(reg);
        platformReadVectorToRegister(state, breg, S(breg).stackpos);
        for (size_t i = breg; i < breg + FPREG_BANK_SIZE; ++i)
            S(i).flags |= REGST_INITIALISED;
    } else if (sz == 1) {
        platformReadScalarToRegister(state, reg, S(reg).stackpos);
        S(reg).flags |= REGST_INITIALISED;
    }
}

static bool flushRegister(jit_compile_state* state, size_t reg)
{
    size_t sz = S(reg).count;
    if (sz == FPREG_BANK_SIZE) {
        int modified = 0;
        size_t breg = thisbank(reg);
        for (size_t i = breg; i < breg + FPREG_BANK_SIZE; ++i)
            modified += (S(i).flags & REGST_MODIFIED);
        // If not modified, we have nothing to do here
        if (modified) {
            platformWriteVectorFromRegister(state, S(breg).stackpos, breg);
            for (size_t i = breg; i < breg + FPREG_BANK_SIZE; ++i)
                S(i).flags &= (~REGST_MODIFIED);
            return true;
        }
    } else if (sz == 1) {
        // If not modified, we have nothing to do here
        if (S(reg).flags & REGST_MODIFIED) {
            platformWriteScalarFromRegister(state, S(reg).stackpos, reg);
            S(reg).flags &= (~REGST_MODIFIED);
            return true;
        }
    }
    return false;
}

static inline void notifyRegisterWritten(jit_compile_state* state, size_t reg, size_t writeSize)
{
    assert(reg + writeSize <= FPREG_MAX);
    for (size_t i = 0; i < writeSize; ++i)
        S(reg+i).flags |= (REGST_INITIALISED | REGST_MODIFIED);
}

static void evictRegister(jit_compile_state* state, size_t reg)
{
    if (S(reg).count == FPREG_BANK_SIZE) {
        size_t breg = thisbank(reg);
        assert((S(breg).flags & REGST_MODIFIED) == 0); // should never be evicting unflushed modified
        JIT_DEBUG_PRINTF(
            "evictRegister: evicting %s%zu, size %zu, was mapped to %d (0x%02X)\n",
            LMNT_FPREG_PREFIX, breg, S(breg).count, S(breg).stackpos, S(breg).stackpos);
        JIT_STATS_PERFORM(++state->stats.reg_evicted);
        for (size_t i = 0; i < FPREG_BANK_SIZE; ++i) {
            S(breg + i).count = 0;
            S(breg + i).flags = REGST_NONE;
        }
    } else if (S(reg).count == 1) {
        assert((S(reg).flags & REGST_MODIFIED) == 0); // should never be evicting unflushed modified
        JIT_DEBUG_PRINTF(
            "evictRegister: evicting %s%zu, size %zu, was mapped to %d (0x%02X)\n",
            LMNT_FPREG_PREFIX, reg, S(reg).count, S(reg).stackpos, S(reg).stackpos);
        JIT_STATS_PERFORM(++state->stats.reg_evicted);
        S(reg).count = 0;
        S(reg).flags = REGST_NONE;
    }
}

static void writeAndEvictRegister(jit_compile_state* state, size_t reg)
{
    flushRegister(state, reg);
    evictRegister(state, reg);
}

static void writeAndEvictRegisters(jit_compile_state* state, size_t reg, size_t scount)
{
    size_t evicted = 0;
    while (evicted < scount) {
        size_t sz = (S(reg + evicted).count > 0 ? S(reg + evicted).count : 1);
        writeAndEvictRegister(state, reg + evicted);
        evicted += sz;
    }
}

#undef S

static overlap_type checkForOverlap(jit_compile_state* state, lmnt_offset spos, size_t scount, size_t* reg, int* importance)
{
    for (size_t i = state->fpreg->start; i < state->fpreg->end; ++i)
    {
        const reg_status s = state->fpreg->s[i];
        if (s.count == 0) continue;
        // Don't check subsequent elements of vectors
        if (s.index != 0) continue;
        if ((s.stackpos >= spos && s.stackpos < spos + scount) || (spos >= s.stackpos && spos < s.stackpos + s.count))
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
            } else if (spos > s.stackpos && spos + scount <= s.stackpos + s.count) {
                *reg += (spos - s.stackpos);
                return OVERLAP_SUBSET;
            } else if (spos < s.stackpos && spos + scount >= s.stackpos + s.count) {
                return OVERLAP_SUPERSET;
            } else {
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
        const reg_status s1 = state->fpreg->s[i];
        if (s1.count == 0) {
            if (s1.flags != REGST_NONE || s1.index != 0) {
                JIT_DEBUG_PRINTF(
                    "validateRegCache: ERROR - %s%zu count = 0 but flags = %d, index = %zu, stackpos = %u\n",
                    LMNT_FPREG_PREFIX, i, s1.flags, s1.index, s1.stackpos);
                return LMNT_ERROR_INTERNAL;
            }
            continue;
        }
        size_t bstart = (s1.count > 1) ? thisbank(i) : i;
        size_t bend = (s1.count > 1) ? nextbank(i+1) : i + 1;
        for (size_t j = state->fpreg->start; j < state->fpreg->end; ++j)
        {
            // These tests will erroneously fail for regs in the same bank
            if (j >= bstart && j < bend) continue;
            const reg_status s2 = state->fpreg->s[j];
            if (s2.count == 0) continue;

            if ((s1.stackpos - s1.index) >= (s2.stackpos - s2.index) && (s1.stackpos - s1.index) < (s2.stackpos - s2.index) + s2.count) {
                JIT_DEBUG_PRINTF(
                    "validateRegCache: ERROR - %s%zu (0x%02X+%zu) overlap with %s%zu (0x%02X+%zu)\n",
                    LMNT_FPREG_PREFIX, j, s2.stackpos, s2.count,
                    LMNT_FPREG_PREFIX, i, s1.stackpos, s1.count);
                return LMNT_ERROR_INTERNAL;
            }
            if ((s2.stackpos - s2.index) >= (s1.stackpos - s1.index) && (s2.stackpos - s2.index) < (s1.stackpos - s1.index) + s1.count) {
                JIT_DEBUG_PRINTF(
                    "validateRegCache: ERROR - %s%zu (0x%02X+%zu) overlap with %s%zu (0x%02X+%zu)\n",
                    LMNT_FPREG_PREFIX, i, s1.stackpos, s1.count,
                    LMNT_FPREG_PREFIX, j, s2.stackpos, s2.count);
                return LMNT_ERROR_INTERNAL;
            }
        }

        if (s1.index > i) {
            JIT_DEBUG_PRINTF(
                "validateRegCache: ERROR - %s%zu claims to be index %zu",
                LMNT_FPREG_PREFIX, i, s1.index);
            return LMNT_ERROR_INTERNAL;
        }
        if (s1.count > 1) {
            for (size_t j = 1; j <= s1.index; ++j) {
                const reg_status s2 = state->fpreg->s[i - j];
                if (s2.count != s1.count) {
                    JIT_DEBUG_PRINTF(
                        "validateRegCache: ERROR - %s%zu (index %zu) count %zu != %s%zu (index %zu) count %zu",
                        LMNT_FPREG_PREFIX, i, s1.index, s1.count,
                        LMNT_FPREG_PREFIX, j, s2.index, s2.count);
                    return LMNT_ERROR_INTERNAL;
                }
                if (s2.index + j != s1.index) {
                    JIT_DEBUG_PRINTF(
                        "validateRegCache: ERROR - %s%zu index %zu not in line with %s%zu index %zu",
                        LMNT_FPREG_PREFIX, i, s1.index,
                        LMNT_FPREG_PREFIX, j, s2.index);
                    return LMNT_ERROR_INTERNAL;
                }
            }
            for (size_t j = 1; s1.index + j < s1.count; ++j) {
                const reg_status s2 = state->fpreg->s[i + j];
                if (s2.count != s1.count) {
                    JIT_DEBUG_PRINTF(
                        "validateRegCache: ERROR - %s%zu (index %zu) count %zu != %s%zu (index %zu) count %zu",
                        LMNT_FPREG_PREFIX, i, s1.index, s1.count,
                        LMNT_FPREG_PREFIX, j, s2.index, s2.count);
                    return LMNT_ERROR_INTERNAL;
                }
                if (s1.index + j != s2.index) {
                    JIT_DEBUG_PRINTF(
                        "validateRegCache: ERROR - %s%zu index %zu not in line with %s%zu index %zu",
                        LMNT_FPREG_PREFIX, i, s1.index,
                        LMNT_FPREG_PREFIX, j, s2.index);
                    return LMNT_ERROR_INTERNAL;
                }
            }
        }
    }

    return LMNT_OK;
}
#endif

#endif