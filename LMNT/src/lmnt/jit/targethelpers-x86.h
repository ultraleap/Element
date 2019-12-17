#ifndef LMNT_JIT_TARGETHELPERS_X86_H
#define LMNT_JIT_TARGETHELPERS_X86_H

#include <stdint.h>
#include <stdlib.h>
#include "lmnt/x86_64/platform.h"
#include "lmnt/jit/jithelpers.h"


// Required for __cpuid
#if defined(_WIN32)
#include <intrin.h>
#endif

//
// CPUID to get runtime flags
//
typedef struct
{
    int eax, ebx, ecx, edx;
} cpuid_regs;

static inline void get_x86_cpuid(uint32_t i, cpuid_regs* exx_registers)
{
#if defined(_WIN32)
    __cpuid((int *)exx_registers, (int)i);
#else
    asm volatile(
#if defined(__PIC__) && defined(LMNT_ARCH_X86_32)
        "mov %%ebx, %%edi;"
        "cpuid;"
        "xchgl %%ebx, %%edi;"
        : "=a"(exx_registers->eax),
        "=D"(exx_registers->ebx),
#else
        "cpuid"
        : "=a"(exx_registers->eax),
        "=b"(exx_registers->ebx),
#endif
        "=c"(exx_registers->ecx),
        "=d"(exx_registers->edx)
        : "a"(i),
        "c"(0));
#endif
}

static inline uint32_t get_x86_xgetbv(int32_t ecx)
{
    uint32_t xcr_low = 0;
#if defined(LMNT_COMPILER_MSVC) && defined(_XCR_XFEATURE_ENABLED_MASK)
    xcr_low = (uint32_t)(_xgetbv(_XCR_XFEATURE_ENABLED_MASK));
#elif defined(LMNT_COMPILER_MSVC) && defined(LMNT_ARCH_X86_32)
    __asm {
        xor        ecx, ecx // xcr 0
        _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0 // For VS2010 and earlier.
        mov        xcr_low, eax
    }
#elif defined(LMNT_ARCH_X86)
    asm(".byte 0x0f, 0x01, 0xd0"
        : "=a"(xcr_low)
        : "c"(0)
        : "%edx");
#endif
    return xcr_low;
}


enum
{
    SIMD_X86_SSE1     = (1 << 0),
    SIMD_X86_SSE2     = (1 << 1),
    SIMD_X86_SSE3     = (1 << 2),
    SIMD_X86_SSSE3    = (1 << 3),
    SIMD_X86_SSE41    = (1 << 4),
    SIMD_X86_SSE42    = (1 << 5),
    SIMD_X86_AVX1     = (1 << 6),
    SIMD_X86_AVX2     = (1 << 7),
    SIMD_X86_AVX2FMA3 = (1 << 8),
};

static cpu_flags get_x86_cpu_flags()
{
    cpu_flags flags = 0;
    cpuid_regs regs;
    get_x86_cpuid(0, &regs);
    int eax0 = regs.eax;
    if ((eax0 & 0x7FFFFFFF) == 0)
        return flags;

    get_x86_cpuid(1, &regs);
    // Detect SSE
    if ((regs.edx & (1 << 25)) != 0)
        flags |= SIMD_X86_SSE1;
    // Detect SSE2
    if ((regs.edx & (1 << 26)) != 0)
        flags |= SIMD_X86_SSE2;
    // Detect SSE3
    if ((regs.ecx & (1)) != 0)
        flags |= SIMD_X86_SSE3;
    // Detect SSSE3
    if ((regs.ecx & (1 << 9)) != 0)
        flags |= SIMD_X86_SSSE3;
    // Detect SSE4_1
    if ((regs.ecx & (1 << 19)) != 0)
        flags |= SIMD_X86_SSE41;
    // Detect SSE4_2
    if ((regs.ecx & (1 << 20)) != 0)
        flags |= SIMD_X86_SSE42;
    // Detect compatible OS enabled context switching (XSAVE/XRESTORE) for AVX and higher
    if ((regs.ecx & (1 << 26)) != 0 && (regs.ecx & (1 << 27)) != 0)
    {
        // Use XGETBV to query the extended control register (XCR)
        // Bit 1 of the extended control register is for SSE support
        // Bit 2 of the extended control register is for AVX support
        if ((get_x86_xgetbv(0) & 0x6) == 0x6)
        {
            // Detect AVX
            if ((regs.ecx & (1 << 28)) != 0)
                flags |= SIMD_X86_AVX1;
            // Can I call for more extended features?
            if ((eax0 & 0x7FFFFFFF) >= 7)
            {
                cpuid_regs regs7;
                get_x86_cpuid(7, &regs7);
                // Detect AVX2
                if ((regs7.ebx & (1 << 5)) != 0)
                    flags |= SIMD_X86_AVX2;
                // Detect FMA3
                if ((regs.ecx & (1 << 12)) != 0)
                    flags |= SIMD_X86_AVX2FMA3;
            }
        }
    }

    return flags;
}

static inline void print_x86_cpu_flags(cpu_flags flags)
{
    JIT_DEBUG_PRINTF("x86 CPU extensions: ");
    if (flags & SIMD_X86_SSE1) JIT_DEBUG_PRINTF("SSE1");
    if (flags & SIMD_X86_SSE2) JIT_DEBUG_PRINTF(", SSE2");
    if (flags & SIMD_X86_SSE3) JIT_DEBUG_PRINTF(", SSE3");
    if (flags & SIMD_X86_SSSE3) JIT_DEBUG_PRINTF(", SSSE3");
    if (flags & SIMD_X86_SSE41) JIT_DEBUG_PRINTF(", SSE4.1");
    if (flags & SIMD_X86_SSE42) JIT_DEBUG_PRINTF(", SSE4.2");
    if (flags & SIMD_X86_AVX1) JIT_DEBUG_PRINTF(", AVX1");
    if (flags & SIMD_X86_AVX2) JIT_DEBUG_PRINTF(", AVX2");
    if (flags & SIMD_X86_AVX2FMA3) JIT_DEBUG_PRINTF(", FMA3");
    if (flags == 0) JIT_DEBUG_PRINTF("(none)");
    JIT_DEBUG_PRINTF("\n");
}

#endif