#ifndef LMNT_JIT_HOSTHELPERS_H
#define LMNT_JIT_HOSTHELPERS_H

#include "lmnt/platform.h"


#if defined(LMNT_ARCH_X86)
//
// Used to protect the memory we write to and execute from
//
#if _WIN32
#include <Windows.h>
#else
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif


static inline void* hostAllocMemory(size_t sz)
{
#ifdef _WIN32
    return VirtualAlloc(0, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#else
    return mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
}

static inline void hostProtectMemory(void* buf, size_t sz)
{
#ifdef _WIN32
    { DWORD dwOld; VirtualProtect(buf, sz, PAGE_EXECUTE_READ, &dwOld); }
#else
    mprotect(buf, sz, PROT_READ | PROT_EXEC);
#endif
}

static inline void hostFreeCompiledBuffer(void* buf, size_t len)
{
#ifdef _WIN32
    VirtualFree(buf, 0, MEM_RELEASE);
#else
    munmap(buf, len);
#endif
}
#endif


#if defined(LMNT_ARCH_ARM)
//
// Used to protect the memory we write to and execute from
//
#ifdef __linux__
#include <sys/mman.h>
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

static inline void* hostAllocMemory(size_t sz)
{
#ifdef __linux__
    return mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
    return malloc(sz);
#endif
}

static inline void hostProtectMemory(void* buf, size_t sz)
{
#ifdef __linux__
    mprotect(buf, sz, PROT_READ | PROT_EXEC);
#endif
}

static inline void hostFreeCompiledBuffer(void* buf, size_t len)
{
#ifdef __linux__
    munmap(buf, len);
#else
    free(buf);
#endif
}
#endif

#endif
