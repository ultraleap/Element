#if !defined(LMNT_JITCONFIG_H)
#define LMNT_JITCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmnt/platform.h"

//
// JIT settings
//

#define LMNT_JIT_COLLECT_STATS
// #define LMNT_JIT_DEBUG_PRINT
// #define LMNT_JIT_DEBUG_NO_REGCACHE
// #define LMNT_JIT_DEBUG_VALIDATE_REGCACHE

#if defined(LMNT_JIT_MEMORY_HEADER)
#include LMNT_JIT_MEMORY_HEADER
#endif

// Signature: void* fn(size_t)
#if !defined(LMNT_JIT_ALLOC_CFN_MEMORY)
#define LMNT_JIT_ALLOC_CFN_MEMORY(sz) hostAllocMemory(sz)
#endif

// Signature: void fn(void*, size_t)
#if !defined(LMNT_JIT_PROTECT_CFN_MEMORY)
#define LMNT_JIT_PROTECT_CFN_MEMORY(buf, sz) hostProtectMemory((buf), (sz))
#endif

// Signature: void fn(void*, size_t)
#if !defined(LMNT_JIT_FREE_CFN_MEMORY)
#define LMNT_JIT_FREE_CFN_MEMORY(buf, sz) hostFreeCompiledBuffer((buf), (sz))
#endif


#ifdef __cplusplus
}
#endif

#endif