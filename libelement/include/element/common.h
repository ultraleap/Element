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
typedef enum message_code_legacy
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
} message_code_legacy;

typedef enum message_code
{
    //todo:: use fun column selection to remove todo prefix once we've moved all the errors over okay
    TODO_ELEMENT_OK = 0,

    //errors specific to libelement should be negative for now, but ideally covered in the Messages.toml
    
    TODO_ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END = -1,
    TODO_ELEMENT_ERROR_EXCEPTION = -2,

    TODO_ELEMENT_ERROR_SERIALISATION = 1,
    TODO_ELEMENT_ERROR_MULTIPLE_DEFINITIONS = 2,
    TODO_ELEMENT_ERROR_INVALID_COMPILE_TARGET = 3,
    TODO_ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED = 4,
    TODO_ELEMENT_ERROR_LOCAL_SHADOWING = 5,
    TODO_ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH = 6,
    TODO_ELEMENT_ERROR_IDENTIFIER_NOT_FOUND = 7,
    TODO_ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED = 8,
    TODO_ELEMENT_ERROR_PARSE = 9,
    TODO_ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE = 10,
    TODO_ELEMENT_ERROR_CIRCULAR_COMPILATION = 11,
    TODO_ELEMENT_ERROR_BOUNDARY_CONVERTER_MISSING = 12,
    TODO_ELEMENT_ERROR_MISSING_PORTS = 13,
    TODO_ELEMENT_ERROR_TYPE_ERROR = 14,
    TODO_ELEMENT_ERROR_INVALID_IDENTIFIER = 15,
    TODO_ELEMENT_ERROR_INVALID_EXPRESSION = 16,
    TODO_ELEMENT_ERROR_INVALID_RETURN_TYPE = 17,
    TODO_ELEMENT_WARNING_REDUNDANT_QUALIFIER = 18,
    TODO_ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE = 19,
    TODO_ELEMENT_ERROR_INTRINSIC_CANNOT_HAVE_BODY = 20,
    TODO_ELEMENT_ERROR_MISSING_FUNCTION_BODY = 21,
    TODO_ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION = 22,
    TODO_ELEMENT_ERROR_UNKNOWN = 9999,
} message_code;

typedef int32_t element_result;

typedef enum message_stage
{
    ELEMENT_STAGE_INVALID = -1,
    ELEMENT_STAGE_MISC,
    ELEMENT_STAGE_TOKENISER,
    ELEMENT_STAGE_PARSER,
    ELEMENT_STAGE_COMPILER,
    ELEMENT_STAGE_EVALUATOR
} message_stage;

typedef int32_t element_stage;

typedef struct element_log_message element_log_message;

struct element_log_message {
    // determines which values in this struct will be relevant
    element_result message_code;
    // the first character of the source file which the message is relevant, or -1
    int column;
    // the length of the relevant part of the source file (starting from the column), or -1
    int length;
    // the line in which the error occured, or -1
    int line;
    // which stage of libelement emitted this message
    element_stage stage;
    // description of the error
    const char* message;
    // separate but related log messages. e.g. a callstack for cascading errors, or null
    element_log_message* related_log_message;
};

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