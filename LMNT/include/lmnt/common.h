#ifndef LMNT_COMMON_H
#define LMNT_COMMON_H

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include "lmnt/config.h"

// Possible enum values for lmnt_result
enum
{
    LMNT_OK                     =   0,
    LMNT_INTERRUPTED            =  -1,
    LMNT_ERROR_INVALID_PTR      =  -2,
    LMNT_ERROR_INVALID_SIZE     =  -3,
    LMNT_ERROR_INVALID_ARCHIVE  =  -4,
    LMNT_ERROR_STACK_DEPTH      =  -5,
    LMNT_ERROR_STACK_SIZE       =  -6,
    LMNT_ERROR_STACK_IN_USE     =  -7,
    LMNT_ERROR_ARGS_MISMATCH    =  -8,
    LMNT_ERROR_RVALS_MISMATCH   =  -9,
    LMNT_ERROR_DEF_MISMATCH     = -10,
    LMNT_ERROR_NOT_FOUND        = -11,
    LMNT_ERROR_NO_IMPL          = -12,
    LMNT_ERROR_ACCESS_VIOLATION = -13,
    LMNT_ERROR_MEMORY_SIZE      = -14,
    LMNT_ERROR_MISSING_EXTCALL  = -15,
    LMNT_ERROR_INTERNAL         = -16,
};

// Possible enum values for lmnt_validation_result
enum
{
    LMNT_VALIDATION_OK           =   0,
    LMNT_VERROR_STRING_HEADER    =  -1,
    LMNT_VERROR_STRING_SIZE      =  -2,
    LMNT_VERROR_STRING_DATA      =  -3,
    LMNT_VERROR_DEF_HEADER       =  -4,
    LMNT_VERROR_DEF_FLAGS        =  -5,
    LMNT_VERROR_DEF_SIZE         =  -6,
    LMNT_VERROR_DEF_CYCLIC       =  -7,
    LMNT_VERROR_ACCESS_VIOLATION =  -8,
    LMNT_VERROR_BAD_INSTRUCTION  =  -9,
    LMNT_VERROR_CODE_HEADER      = -10,
    LMNT_VERROR_CODE_SIZE        = -11,
    LMNT_VERROR_HEADER_MAGIC     = -12,
    LMNT_VERROR_SEGMENTS_SIZE    = -13,
    LMNT_VERROR_CONSTANTS_ALIGN  = -14,
    LMNT_VERROR_STACK_SIZE       = -15,
    LMNT_VERROR_STACK_DEPTH      = -16,
    LMNT_VERROR_NO_IMPL          = -17, // get rid of this at some point
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
struct lmnt_ictx_s;
typedef struct lmnt_ictx_s lmnt_ictx;

#define LMNT_OK_OR_RETURN(t) \
{ \
    const lmnt_result ok_or_return_result = (t); \
    if (ok_or_return_result != LMNT_OK) \
        return ok_or_return_result; \
}

#define LMNT_V_OK_OR_RETURN(t) \
{ \
    const lmnt_validation_result ok_or_return_result = (t); \
    if (ok_or_return_result != LMNT_VALIDATION_OK) \
        return ok_or_return_result; \
}

#define LMNT_COMBINE_OFFSET(lo, hi) (lo | (hi << sizeof(lmnt_offset) * CHAR_BIT))

// MSVC does not currently include the C11-standard _Static_assert, only the C++-style variant
#if defined(_MSC_VER)
#define _Static_assert static_assert
#endif

#if defined(LMNT_COMPILER_GCC) || defined(LMNT_COMPILER_CLANG)
#define LMNT_LIKELY(expr)    (__builtin_expect(!!(expr), 1))
#define LMNT_UNLIKELY(expr)  (__builtin_expect(!!(expr), 0))
#else
#define LMNT_LIKELY(expr) (expr)
#define LMNT_UNLIKELY(expr) (expr)
#endif

#endif
