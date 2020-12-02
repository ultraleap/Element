#if !defined(ELEMENT_COMMON_H)
#define ELEMENT_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief value type
 */
typedef float element_value;

/**
 * @brief result type
 */
typedef enum element_result
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
    ELEMENT_ERROR_MISSING_FUNCTION_RETURN = 31,
    ELEMENT_ERROR_STRUCT_PORTLIST_CONTAINS_DISCARDS = 32,
    ELEMENT_ERROR_DEFAULT_ARGUMENT_NOT_AT_END = 33,
    ELEMENT_ERROR_INFINITE_LOOP = 35, //todo: we use this as if it was ELEMENT_ERROR_COMPILETIME_LOOP_TOO_MANY_ITERATIONS
    ELEMENT_ERROR_INVALID_CALL_NONFUNCTION = 36,
    ELEMENT_ERROR_NOT_INDEXABLE = 37,
    ELEMENT_ERROR_NOT_A_CONSTRAINT = 38,
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
    ELEMENT_ERROR_INVALID_FILE_TYPE = -103,
    ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END = -200,
    ELEMENT_ERROR_EXCEPTION = -201,
    ELEMENT_ERROR_CONSTRAINT_HAS_BODY = -202,
    ELEMENT_ERROR_BAD_INDEX_INTO_NUMBER = -203,
    ELEMENT_ERROR_BAD_NUMBER_EXPONENT = -204,
    ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL = -205,
    ELEMENT_ERROR_MISSING_SEMICOLON = -206,
    ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL = -207,
    ELEMENT_ERROR_INVALID_CONTENTS_FOR_CALL = -208,
    ELEMENT_ERROR_INVALID_PORT = -209,
    ELEMENT_ERROR_INVALID_TYPENAME = -209,
    ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST = -210,
    ELEMENT_ERROR_RESERVED_IDENTIFIER = -211,
    ELEMENT_ERROR_MISSING_BODY = -212,
    ELEMENT_ERROR_PARTIAL_GRAMMAR = -213,
    ELEMENT_ERROR_STRUCT_INVALID_BODY = -214,
    ELEMENT_ERROR_CONSTRAINT_INVALID_BODY = -215,
    ELEMENT_ERROR_MISSING_COMMA_IN_ANONYMOUS_BLOCK = -216,
    ELEMENT_ERROR_COMPILETIME_LOOP_TOO_MANY_ITERATIONS = -217,
    ELEMENT_ERROR_API_INSTRUCTION_IS_NULL = -10000,
    ELEMENT_ERROR_API_OBJECT_IS_NOT_ERROR = -10001,
    ELEMENT_ERROR_API_OBJECT_IS_NULL = -10002,
    ELEMENT_ERROR_API_PORT_IS_NULL = -10003,
    ELEMENT_ERROR_API_DECLARATION_IS_NULL = -10004,
    ELEMENT_ERROR_API_STRING_IS_NULL = -10005,
    ELEMENT_ERROR_API_INSUFFICIENT_BUFFER = -10006,
    ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL = -10007,
    ELEMENT_ERROR_API_OUTPUT_IS_NULL = -10008,
    ELEMENT_ERROR_API_INVALID_INPUT = -10009,
    ELEMENT_ERROR_API_AST_IS_NULL = -10010,
    ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL = -10011,
    ELEMENT_ERROR_API_PARSER_CTX_IS_NULL = -10012,
    ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL = -10013,
    ELEMENT_ERROR_API_PORTS_IS_NULL = -10014,
    ELEMENT_ERROR_API_UNKNOWN = -20000,
} element_result;

/**
 * @brief compilation stage
 */
typedef enum element_stage
{
    ELEMENT_STAGE_INVALID = -1,
    ELEMENT_STAGE_MISC,
    ELEMENT_STAGE_TOKENISER,
    ELEMENT_STAGE_PARSER,
    ELEMENT_STAGE_COMPILER,
    ELEMENT_STAGE_EVALUATOR
} element_stage;

/**
 * @brief log message
 */
typedef struct element_log_message element_log_message;

/**
 * @brief log message definition
 */
struct element_log_message
{
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
    // error message length
    int message_length;
    // filename
    const char* filename;
    // line of code in the source
    const char* line_in_source;
    // separate but related log messages. e.g. a callstack for cascading errors, or null
    element_log_message* related_log_message;
};

typedef void (*element_log_callback)(const element_log_message*, void*);

#define ELEMENT_OK_OR_RETURN(t)                         \
    do                                                  \
    {                                                   \
        const element_result ok_or_return_result = (t); \
        if (ok_or_return_result != ELEMENT_OK)          \
            return ok_or_return_result;                 \
    } while (0)

#if defined(__cplusplus)
}
#endif
#endif