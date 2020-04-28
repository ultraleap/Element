#if !defined(ELEMENT_COMMON_H)
#define ELEMENT_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef float element_value;

typedef struct element_ast_ctx element_ast_ctx;

typedef struct
{
    element_ast_ctx* ast;
} element_ctx;


// Possible enum values for element_result
enum
{
    ELEMENT_OK = 0,
    ELEMENT_INTERRUPTED = -1,
    ELEMENT_ERROR_INVALID_PTR = -2,
    ELEMENT_ERROR_INVALID_SIZE = -3,
    ELEMENT_ERROR_INVALID_ARCHIVE = -4,
    ELEMENT_ERROR_ARGS_MISMATCH = -8,
    ELEMENT_ERROR_RVALS_MISMATCH = -9,
    ELEMENT_ERROR_DEF_MISMATCH = -10,
    ELEMENT_ERROR_NOT_FOUND = -11,
    ELEMENT_ERROR_NO_IMPL = -12,
    ELEMENT_ERROR_ACCESS_VIOLATION = -13,
    ELEMENT_ERROR_MEMORY_SIZE = -14,
    ELEMENT_ERROR_MISSING_EXTCALL = -15,
    ELEMENT_ERROR_INVALID_OPERATION = -16,
    ELEMENT_ERROR_PRELUDE_ALREADY_LOADED = -100,
    ELEMENT_ERROR_DIRECTORY_NOT_FOUND = -101,
    ELEMENT_ERROR_FILE_NOT_FOUND = -102,
};
typedef int32_t element_result;


#define ELEMENT_OK_OR_RETURN(t) \
{ \
    const element_result ok_or_return_result = (t); \
    if (ok_or_return_result != ELEMENT_OK) \
        return ok_or_return_result; \
}

#if defined(__cplusplus)
}
#endif
#endif