#ifndef LMNT_JIT_TARGETHELPERS_ARM_H
#define LMNT_JIT_TARGETHELPERS_ARM_H

#include <stdint.h>
#include <stdlib.h>
#include "lmnt/arm/platform.h"
#include "jit/jithelpers.h"


enum
{
    SIMD_ARM_NEON  = (1 << 0),
};

static inline cpu_flags get_arm_cpu_flags()
{
    // TODO: spot NEON
    return 0;
}

static inline void print_arm_cpu_flags(cpu_flags flags)
{
    JIT_DEBUG_PRINTF("ARM CPU extensions: ");
    if (flags & SIMD_ARM_NEON) JIT_DEBUG_PRINTF("NEON");
    if (flags == 0) JIT_DEBUG_PRINTF("(none)");
    JIT_DEBUG_PRINTF("\n");
}

#endif