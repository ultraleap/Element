#ifndef LMNT_PLATFORM_H
#define LMNT_PLATFORM_H

#if defined(_M_X64) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64)
#define LMNT_ARCH_X86
#define LMNT_ARCH_X86_64
#define LMNT_ARCHBITS 64
#elif defined(_X86_) || defined(_M_IX86) || defined(__i386__) || defined(i386)
#define LMNT_ARCH_X86
#define LMNT_ARCH_X86_32
#define LMNT_ARCHBITS 32
#elif defined(__aarch64__) || defined(_M_ARM64)
#define LMNT_ARCH_ARM
#define LMNT_ARCH_ARM64
#define LMNT_ARCHBITS 64
#elif defined(__arm__) || defined(_M_ARM)
#define LMNT_ARCH_ARM
#define LMNT_ARCHBITS 32
#if defined(__thumb__)
#define LMNT_ARCH_ARMTHUMB
#endif
#if defined(__ARM_ARCH_7A__)
#define LMNT_ARCH_ARMV7A
#endif
#if defined(__ARM_ARCH_7M__)
#define LMNT_ARCH_ARMv7M
#endif
#else
#define LMNT_ARCH_UNKNOWN
#endif

// Compilers
#if defined(_MSC_VER)
#define LMNT_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#define LMNT_COMPILER_GCC __GNUC__
#elif defined(__clang__)
#define LMNT_COMPILER_CLANG __clang__
#endif



#if defined(LMNT_ARCH_X86_64)
#include "lmnt/x86_64/platform.h"
#endif
#if defined(LMNT_ARCH_ARM)
#include "lmnt/arm/platform.h"
#endif

#endif