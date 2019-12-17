#ifndef LMNT_X86_64_PLATFORM_H
#define LMNT_X86_64_PLATFORM_H

#if !defined(LMNT_MEMCPY)
    #define LMNT_MEMORY_HEADER "lmnt/x86_64/memcpy_fast.h"
    #define LMNT_MEMCPY lmnt_memcpy_fast
#endif

#define LMNT_JIT_TARGET_NATIVE LMNT_JIT_TARGET_X86_64

// Enable lookahead
#define LMNT_JIT_X86_64_ENABLE_LOOKAHEAD

// Allow use of non-volatile registers?
#define LMNT_JIT_X86_64_ALLOW_NV_REGISTERS

#endif