#if !defined(ELEMENT_INTERPRETER_OBJECT_MODEL_H)
#define ELEMENT_INTERPRETER_OBJECT_MODEL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/interpreter.h"

/// <summary>
///
/// </summary>
/// <param name="declaration"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_declaration_to_object(
    const element_declaration* declaration,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="context"></param>
/// <param name="compilable"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_compile(
    element_interpreter_ctx* context,
    const element_object* compilable,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="context"></param>
/// <param name="callable"></param>
/// <param name="arguments"></param>
/// <param name="arguments_count"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_call(
    element_interpreter_ctx* context,
    const element_object* object,
    const element_object* arguments,
    unsigned int arguments_count,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="context"></param>
/// <param name="indexable"></param>
/// <param name="index"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_index(
    element_interpreter_ctx* context,
    const element_object* object,
    const char* index,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="object"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_to_instruction(
    const element_object* object,
    element_instruction** output);

struct element_error
{
    element_result message_code;
    char* message;
};

//object must be an error, and it must be kept alive while using the log message
element_result element_object_to_log_message(
    const element_object* object,
    element_log_message* output);

typedef struct
{
    int line = 0;
    int character_start = 0;
    int character_end = 0;

    char* filename = NULL; //must be deleted by user
    char* line_in_source = NULL; //must be deleted by user
    char* text = NULL; //must be deleted by user
} element_source_information;

element_result element_object_get_source_information(
    const element_object* object,
    element_source_information* output);

element_result element_object_get_typeof(
    const element_object* object,
    char* buffer,
    int buffer_size);

#if defined(__cplusplus)
}
#endif
#endif