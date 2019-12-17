#ifndef LMNT_ARM_PLATFORM_H
#define LMNT_ARM_PLATFORM_H

// #if !defined(LMNT_MEMCPY)
// #define LMNT_MEMORY_HEADER "some_arm_fast_memcpy.h"
// #define LMNT_MEMCPY lmnt_memcpy_fast
// #endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define LMNT_JIT_TARGET_NATIVE LMNT_JIT_TARGET_ARM64
#elif defined(__ARM_ARCH_7A__)
#define LMNT_JIT_TARGET_NATIVE LMNT_JIT_TARGET_ARMV7A
#elif defined(__ARM_ARCH_7M__)
#define LMNT_JIT_TARGET_NATIVE LMNT_JIT_TARGET_ARMV7M
#endif

// Enable lookahead
#define LMNT_JIT_ARM_ENABLE_LOOKAHEAD

// Allow use of non-volatile registers?
#define LMNT_JIT_ARM_ALLOW_NV_REGISTERS

#endif
