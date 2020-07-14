#ifndef LMNT_JITHELPERS_H
#define LMNT_JITHELPERS_H

#include <stdlib.h>
#include <stdbool.h>
#include "lmnt/config.h"
#include "lmnt/jitconfig.h"
#include "lmnt/interpreter.h"
#include "lmnt/jit.h"
#include "lmnt/jit/hosthelpers.h"

#ifndef DASM_FDECL
#define DASM_FDECL static
#endif
#ifndef DASM_FDEF
#define DASM_FDEF static
#endif
#include "dasm_proto.h"

#if defined(LMNT_JIT_COLLECT_STATS)
#define JIT_STATS_PERFORM(...) __VA_ARGS__;
#else
#define JIT_STATS_PERFORM(...) do {} while(0)
#endif

#if defined(LMNT_JIT_DEBUG_PRINT)
#define JIT_DEBUG_PRINTF(...) LMNT_PRINTF(__VA_ARGS__)
#else
#define JIT_DEBUG_PRINTF(...) do {} while(0)
#endif

#define LMNT_FPREG_PREFIX "fp"

typedef enum
{
    REGST_NONE = (0 << 0),
    REGST_MODIFIED = (1 << 0),
    REGST_INITIALISED = (1 << 1),
} reg_status_flags;

typedef enum
{
    OVERLAP_NONE,
    OVERLAP_ALIGNED,
    OVERLAP_ALIGNED_SUBSET,
    OVERLAP_ALIGNED_SUPERSET,
    OVERLAP_SUBSET,
    OVERLAP_SUPERSET,
    OVERLAP_UNALIGNED,
} overlap_type;

typedef unsigned int cpu_flags;
struct jit_fpreg_data_s;
typedef struct jit_fpreg_data_s jit_fpreg_data;

typedef struct
{
    dasm_State* dasm_state;
    const lmnt_instruction* instructions;
    lmnt_loffset in_count;
    lmnt_loffset cur_in;
    jit_fpreg_data* fpreg;
    cpu_flags cpuflags;
#if defined(LMNT_JIT_COLLECT_STATS)
    lmnt_jit_compile_stats stats;
#endif
} jit_compile_state;



// prototypes for platform-specific functions implemented in dasc
static void platformWriteAndEvictAll(jit_compile_state* state);
static void platformWriteAndEvictVolatile(jit_compile_state* state);
static void platformReadScalarToRegister(jit_compile_state* state, size_t reg, lmnt_offset stackpos);
static void platformWriteScalarFromRegister(jit_compile_state* state, lmnt_offset stackpos, size_t reg);
static void platformReadVectorToRegister(jit_compile_state* state, size_t reg, lmnt_offset stackpos);
static void platformWriteVectorFromRegister(jit_compile_state* state, lmnt_offset stackpos, size_t reg);


typedef enum
{
    ACCESSTYPE_READ = (1 << 0),
    ACCESSTYPE_WRITE = (1 << 1),
} access_type;


static inline size_t getAccessSize(lmnt_offset inst, int arg)
{
    lmnt_operand_type optype;
    switch (arg)
    {
    case 1: optype = lmnt_opcode_info[inst].operand1; break;
    case 2: optype = lmnt_opcode_info[inst].operand2; break;
    case 3: optype = lmnt_opcode_info[inst].operand3; break;
    default: return 0;
    }
    switch (optype)
    {
    case LMNT_OPERAND_STACK1: return 1;
    case LMNT_OPERAND_STACK4: return 4;
    case LMNT_OPERAND_STACKN:
    default: return 0;  // no stack access or can't be predicted
    }
}

#define LOOKAHEAD_SIZE 8
#define LOOKAHEAD_READ_WEIGHT 4
#define LOOKAHEAD_WRITE_WEIGHT 4
static int lookahead(jit_compile_state* state, lmnt_offset spos, size_t scount)
{
    int count = 0;
    for (size_t i = state->cur_in + 1; i <= state->cur_in + LOOKAHEAD_SIZE && i < state->in_count; ++i)
    {
        size_t arg1size = getAccessSize(state->instructions[i].opcode, 1);
        size_t arg2size = getAccessSize(state->instructions[i].opcode, 2);
        size_t arg3size = getAccessSize(state->instructions[i].opcode, 3);
        if (state->instructions[i].arg1 == spos && arg1size > 0 && arg1size <= scount)
            count += LOOKAHEAD_READ_WEIGHT;
        if (state->instructions[i].arg2 == spos && arg2size > 0 && arg2size <= scount)
            count += LOOKAHEAD_READ_WEIGHT;
        if (state->instructions[i].arg3 == spos && arg3size > 0 && arg3size <= scount)
            count += LOOKAHEAD_WRITE_WEIGHT;
    }
    return count;
}

static lmnt_result targetLinkAndEncode(dasm_State** d, void** buf, size_t* sz)
{
    if (dasm_link(d, sz) != DASM_S_OK) return LMNT_ERROR_INTERNAL;
    *buf = LMNT_JIT_ALLOC_CFN_MEMORY(*sz);
    if (*buf) {
        if (dasm_encode(d, *buf) == DASM_S_OK) {
            LMNT_JIT_PROTECT_CFN_MEMORY(*buf, *sz);
            return LMNT_OK;
        } else {
            LMNT_JIT_FREE_CFN_MEMORY(*buf, *sz);
            *buf = NULL;
            *sz = 0;
            return LMNT_ERROR_INTERNAL;
        }
    } else {
        return LMNT_ERROR_MEMORY_SIZE;
    }
}


// Target-specific implementations
static bool allowIndividualLaneAccess(jit_compile_state* state);
static bool chooseExistingScalarRegister(jit_compile_state* state, size_t* reg, int importance);
static bool chooseExistingVectorRegister(jit_compile_state* state, size_t* reg, int importance);
static inline void updateRegister(jit_compile_state* state, size_t reg, lmnt_offset spos, size_t scount, int importance, access_type modified);
static inline void updateRegisterState(jit_compile_state* state, size_t reg, int importance, access_type modified);
static inline size_t getRegisterCount(jit_compile_state* state, size_t reg);
static inline bool isRegisterInitialised(jit_compile_state* state, size_t reg);
static void initialiseRegister(jit_compile_state* state, size_t reg);
static bool flushRegister(jit_compile_state* state, size_t reg);
static void flushAllRegisters(jit_compile_state* state);
static inline void notifyRegisterWritten(jit_compile_state* state, size_t reg, size_t writeSize);
static void writeAndEvictRegister(jit_compile_state* state, size_t reg);
static void writeAndEvictRegisters(jit_compile_state* state, size_t reg, size_t scount);
static overlap_type checkForOverlap(jit_compile_state* state, lmnt_offset spos, size_t scount, size_t* reg, int* importance);


static bool acquireScalarRegister(jit_compile_state* state, lmnt_offset spos, size_t* reg, access_type access)
{
#if !defined(LMNT_JIT_DEBUG_NO_REGCACHE)
    const size_t scount = 1;
    int importance = lookahead(state, spos, scount);
    JIT_DEBUG_PRINTF(
        "acquireScalarRegister: getting reg for stack pos %u (0x%02X), importance %d\n",
        spos, spos, importance);
    size_t overlap_reg;
    int overlap_importance;
    overlap_type overlap;
    do
    {
        overlap = checkForOverlap(state, spos, scount, &overlap_reg, &overlap_importance);
        if (overlap == OVERLAP_ALIGNED || overlap == OVERLAP_ALIGNED_SUBSET)
        {
            // Happy days
            *reg = overlap_reg;
            if (!isRegisterInitialised(state, *reg))
                initialiseRegister(state, *reg);
            updateRegisterState(state, *reg, importance, access);
            JIT_STATS_PERFORM(++state->stats.reg_aligned);
            JIT_DEBUG_PRINTF(
                "acquireScalarRegister: aligned, returning existing %s%zu\n",
                LMNT_FPREG_PREFIX, *reg);
            return true;
        }
        else if (overlap == OVERLAP_SUBSET && allowIndividualLaneAccess(state))
        {
            // Allow using a subset of an existing vector register
            *reg = overlap_reg;
            if (!isRegisterInitialised(state, *reg))
                initialiseRegister(state, *reg);
            updateRegisterState(state, *reg, importance, access);
            JIT_DEBUG_PRINTF(
                "acquireScalarRegister: unaligned subset, returning existing %s%zu\n",
                LMNT_FPREG_PREFIX, *reg);
            return true;
        }
        else if (overlap != OVERLAP_NONE)
        {
            JIT_DEBUG_PRINTF(
                "acquireScalarRegister: unaligned, deferring to (and flushing) more important %s%zu\n",
                LMNT_FPREG_PREFIX, overlap_reg);
            // Make sure this is flushed if applicable so our subsequent read is not stale
            flushRegister(state, overlap_reg);
            return false;
        }
    } while (overlap != OVERLAP_NONE);

    if (chooseExistingScalarRegister(state, reg, importance))
    {
        writeAndEvictRegisters(state, *reg, scount);
        updateRegister(state, *reg, spos, scount, importance, access);
        // Caller is going to expect the correct data to be in here, make sure this is true
        if (access & ACCESSTYPE_READ)
            initialiseRegister(state, *reg);
        JIT_DEBUG_PRINTF("acquireScalarRegister: acquired new %s%zu\n", LMNT_FPREG_PREFIX, *reg);
        return true;
    }

    JIT_DEBUG_PRINTF("acquireScalarRegister: no match\n");
#endif
    return false;
}

static bool acquireVectorRegister(jit_compile_state* state, lmnt_offset spos, size_t* reg, access_type access)
{
#if !defined(LMNT_JIT_DEBUG_NO_REGCACHE)
    const size_t scount = 4;
    int importance = lookahead(state, spos, scount);
    JIT_DEBUG_PRINTF(
        "acquireVectorRegister: getting reg for stack pos %u (0x%02X), importance %d\n",
        spos, spos, importance);
    size_t overlap_reg;
    int overlap_importance;
    overlap_type overlap;
    do
    {
        overlap = checkForOverlap(state, spos, scount, &overlap_reg, &overlap_importance);
        if (overlap == OVERLAP_ALIGNED || overlap == OVERLAP_ALIGNED_SUBSET)
        {
            // Happy days
            *reg = overlap_reg;
            if (!isRegisterInitialised(state, *reg))
                initialiseRegister(state, *reg);
            updateRegisterState(state, *reg, importance, access);
            JIT_STATS_PERFORM(++state->stats.reg_aligned);
            JIT_DEBUG_PRINTF(
                "acquireVectorRegister: aligned, returning existing %s%zu\n",
                LMNT_FPREG_PREFIX, *reg);
            return true;
        }
        else if (overlap != OVERLAP_NONE)
        {
            // There's an entry with which we're unaligned... decide if we're more important or not
            JIT_STATS_PERFORM(++state->stats.reg_unaligned);
            if (importance > overlap_importance)
            {
                JIT_DEBUG_PRINTF(
                    "acquireVectorRegister: unaligned, evicting existing %s%zu and continuing\n",
                    LMNT_FPREG_PREFIX, overlap_reg);
                writeAndEvictRegister(state, overlap_reg);
                // Now fall back through and check if there are any other conflicts
                continue;
            }
            else
            {
                // TODO: make sure we flush *all* overlapping registers
                JIT_DEBUG_PRINTF(
                    "acquireVectorRegister: unaligned, deferring to (and flushing) more important %s%zu\n",
                    LMNT_FPREG_PREFIX, overlap_reg);
                // Make sure this is flushed if applicable so our subsequent read is not stale
                flushRegister(state, overlap_reg);
                return false;
            }
        }
    } while (overlap != OVERLAP_NONE);

    if (chooseExistingVectorRegister(state, reg, importance))
    {
        writeAndEvictRegisters(state, *reg, scount);
        updateRegister(state, *reg, spos, scount, importance, access);
        // Caller is going to expect the correct data to be in here, make sure this is true
        if (access & ACCESSTYPE_READ)
            initialiseRegister(state, *reg);
        JIT_DEBUG_PRINTF("acquireVectorRegister: acquired new %s%zu\n", LMNT_FPREG_PREFIX, *reg);
        return true;
    }

    JIT_DEBUG_PRINTF("acquireVectorRegister: no match\n");
#endif
    return false;
}

static bool acquireScalarRegisterOrDefault(jit_compile_state* state, lmnt_offset spos, size_t* reg, access_type access, size_t def)
{
    bool result = acquireScalarRegister(state, spos, reg, access);
    if (!result)
        *reg = def;
    return result;
}

static bool acquireVectorRegisterOrDefault(jit_compile_state* state, lmnt_offset spos, size_t* reg, access_type access, size_t def)
{
    bool result = acquireVectorRegister(state, spos, reg, access);
    if (!result)
        *reg = def;
    return result;
}

static bool acquireScalarRegisterOrLoad(jit_compile_state* state, lmnt_offset spos, size_t* reg, access_type access, size_t regToLoad)
{
    bool result = acquireScalarRegister(state, spos, reg, access);
    if (!result) {
        platformReadScalarToRegister(state, regToLoad, spos);
        *reg = regToLoad;
    }
    return result;
}

static bool acquireVectorRegisterOrLoad(jit_compile_state* state, lmnt_offset spos, size_t* reg, access_type access, size_t regToLoad)
{
    bool result = acquireVectorRegister(state, spos, reg, access);
    if (!result) {
        platformReadVectorToRegister(state, regToLoad, spos);
        *reg = regToLoad;
    }
    return result;
}

#undef LMNT_FPREG_PREFIX

#endif
