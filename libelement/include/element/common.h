#if !defined(ELEMENT_COMMON_H)
#define ELEMENT_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef float element_value;

typedef struct element_ast_ctx element_ast_ctx;

typedef struct
{
    element_ast_ctx* ast;
} element_ctx;

typedef enum message_type
{
    //ELEMENT
    ELEMENT_OK = 0,
    ELEMENT_ERROR_SERIALISATION = 1,
    ELEMENT_ERROR_MULTIPLE_DEFINITIONS = 2,
    ELEMENT_ERROR_INVALID_COMPILE_TARGET = 3,
    ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED = 4,
    ELEMENT_ERROR_LOCAL_SHADOWING = 5,
    ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH = 6,
    ELEMENT_ERROR_IDENTIFIER_NOT_FOUND = 7,
    ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED = 8,
    ELEMENT_ERROR_PARSE = 9,
    ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE = 10,
    ELEMENT_ERROR_CIRCULAR_COMPILATION = 11,
    ELEMENT_ERROR_BOUNDARY_CONVERTER_MISSING = 12,
    ELEMENT_ERROR_MISSING_PORTS = 13,
    ELEMENT_ERROR_TYPE_ERROR = 14,
    ELEMENT_ERROR_INVALID_IDENTIFIER = 15,
    ELEMENT_ERROR_INVALID_EXPRESSION = 16,
    ELEMENT_ERROR_INVALID_RETURN_TYPE = 17,
    ELEMENT_WARNING_REDUNDANT_QUALIFIER = 18,
    ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE = 19,
    ELEMENT_ERROR_INTRINSIC_CANNOT_HAVE_BODY = 20,
    ELEMENT_ERROR_MISSING_FUNCTION_BODY = 21,
    ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION = 22,
    ELEMENT_ERROR_UNKNOWN = 9999,

	//INTERNAL
	//Errors specific to libelement should be negative for now, but ideally covered in the Messages.toml

	//LEGACY
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

	//NEW
    ELEMENT_ERROR_PRELUDE_ALREADY_LOADED = -100,
    ELEMENT_ERROR_DIRECTORY_NOT_FOUND = -101,
    ELEMENT_ERROR_FILE_NOT_FOUND = -102,
    ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END = -200,
    ELEMENT_ERROR_EXCEPTION = -201,
    ELEMENT_ERROR_CONSTRAINT_HAS_BODY = -202,
    ELEMENT_ERROR_BAD_INDEX_INTO_NUMBER = -203,
    ELEMENT_ERROR_BAD_NUMBER_EXPONENT = -203,
} message_type;

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
    int character;
    // the length of the relevant part of the source file (starting from the character), or -1
    int length;
    // the line in which the error occured, or -1
    int line;
    // which stage of libelement emitted this message
    element_stage stage;
    // description of the error
    const char* message;
    // filename
    const char* filename;
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