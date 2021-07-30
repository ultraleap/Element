#ifndef LMNT_COMMON_H
#define LMNT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include "lmnt/config.h"

// Possible enum values for lmnt_result
enum
{
    LMNT_OK                       =  0x00,
    LMNT_INTERRUPTED              = -0x01,
    LMNT_ERROR_INVALID_PTR        = -0x02,
    LMNT_ERROR_INVALID_SIZE       = -0x03,
    LMNT_ERROR_INVALID_ARCHIVE    = -0x04,
    LMNT_ERROR_STACK_DEPTH        = -0x10,
    LMNT_ERROR_STACK_SIZE         = -0x11,
    LMNT_ERROR_STACK_IN_USE       = -0x12,
    LMNT_ERROR_ARGS_MISMATCH      = -0x20,
    LMNT_ERROR_RVALS_MISMATCH     = -0x21,
    LMNT_ERROR_DEF_MISMATCH       = -0x22,
    LMNT_ERROR_NOT_FOUND          = -0x30,
    LMNT_ERROR_ACCESS_VIOLATION   = -0x40,
    LMNT_ERROR_MEMORY_SIZE        = -0x41,
    LMNT_ERROR_MISSING_EXTCALL    = -0x50,
    LMNT_ERROR_FEATURE_DISABLED   = -0x51,
    LMNT_ERROR_UNPREPARED_ARCHIVE = -0x60,
    LMNT_ERROR_INTERNAL           = -0xF0,
    LMNT_ERROR_NO_IMPL            = -0xF1,
    // These error codes are only used internally
    LMNT_BRANCHING                = -0x10000,
    LMNT_RETURNING                = -0x10001,
};

// Possible enum values for lmnt_validation_result
enum
{
    LMNT_VALIDATION_OK           =  0x00,
    LMNT_VERROR_STRING_HEADER    = -0x10,
    LMNT_VERROR_STRING_SIZE      = -0x11,
    LMNT_VERROR_STRING_DATA      = -0x12,
    LMNT_VERROR_STRING_ALIGN     = -0x13,
    LMNT_VERROR_DEF_HEADER       = -0x20,
    LMNT_VERROR_DEF_FLAGS        = -0x21,
    LMNT_VERROR_DEF_SIZE         = -0x22,
    LMNT_VERROR_DEF_CYCLIC       = -0x23,
    LMNT_VERROR_DEF_DEFAULT_ARGS = -0x24,
    LMNT_VERROR_ACCESS_VIOLATION = -0x30,
    LMNT_VERROR_BAD_INSTRUCTION  = -0x31,
    LMNT_VERROR_CODE_HEADER      = -0x40,
    LMNT_VERROR_CODE_SIZE        = -0x41,
    LMNT_VERROR_HEADER_MAGIC     = -0x50,
    LMNT_VERROR_SEGMENTS_SIZE    = -0x51,
    LMNT_VERROR_SEGMENTS_ALIGN   = -0x52,
    LMNT_VERROR_STACK_SIZE       = -0x60,
    LMNT_VERROR_STACK_DEPTH      = -0x61,
    LMNT_VERROR_DATA_HEADER      = -0x70,
    LMNT_VERROR_DATA_SIZE        = -0x71,
    LMNT_VERROR_NO_IMPL          = -0xF1, // get rid of this at some point
};

// Result type returned by most library operations
// Note that in some cases, a successful result may generate a positive non-zero value rather than LMNT_OK
typedef int32_t lmnt_result;

// Result type returned by validation operations within the library
// Note that in some cases, a successful result may generate a positive non-zero value rather than LMNT_OK
typedef int32_t lmnt_validation_result;

// Offset type used by smaller archive tables
typedef uint16_t lmnt_offset;

// Offset type used by larger archive tables
typedef uint32_t lmnt_loffset;

// Value type used for arguments, return values and locals (type is configured in config.h or via compile definition)
typedef LMNT_VALUE_TYPE lmnt_value;


//
// Internal and compatibility definitions
//

// Forward declare lmnt_ictx so it can be used in definitions
struct lmnt_ictx;
typedef struct lmnt_ictx lmnt_ictx;

// Add compiler hints to help out branch prediction
#if defined(LMNT_COMPILER_GCC) || defined(LMNT_COMPILER_CLANG)
#define LMNT_LIKELY(expr)    (__builtin_expect(!!(expr), 1))
#define LMNT_UNLIKELY(expr)  (__builtin_expect(!!(expr), 0))
#else
#define LMNT_LIKELY(expr) (expr)
#define LMNT_UNLIKELY(expr) (expr)
#endif

// Convenience macros for checking an operation succeeded
#define LMNT_OK_OR_RETURN(t) \
{ \
    const lmnt_result ok_or_return_result = (t); \
    if (LMNT_UNLIKELY(ok_or_return_result != LMNT_OK)) \
        return ok_or_return_result; \
}

#define LMNT_V_OK_OR_RETURN(t) \
{ \
    const lmnt_validation_result ok_or_return_result = (t); \
    if (LMNT_UNLIKELY(ok_or_return_result != LMNT_VALIDATION_OK)) \
        return ok_or_return_result; \
}

#define LMNT_COMBINE_OFFSET(lo, hi) (lo | (hi << (sizeof(lmnt_offset) * CHAR_BIT)))

#define LMNT_ROUND_UP(n, d) ((n) + ((d) - ((n) % (d))))

// MSVC does not currently include the C11-standard _Static_assert, only the C++-style variant
#if defined(_MSC_VER)
#define _Static_assert static_assert
#endif

// Define hopefully-static asserts that fall back to runtime asserts on pre-C11
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define LMNT_STATIC_ASSERT(expr,message) _Static_assert((expr), message)
#else
#define LMNT_STATIC_ASSERT(expr,message) assert((expr) && (message))
#endif

// Specify force-inline
#if defined(_MSC_VER) && !defined(__clang__)
#define LMNT_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__) || defined(__GNUC_MINOR__)
#define LMNT_FORCEINLINE __attribute__((always_inline))
#else
#warning Unknown platform, no force-inline support
#define LMNT_FORCEINLINE
#endif

#ifdef __cplusplus
}
#endif

#endif
