#if !defined(ELEMENT_INTERPRETER_OBJECT_H)
#define ELEMENT_INTERPRETER_OBJECT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/interpreter.h"

typedef struct element_object element_object;

void element_delete_object(element_object** object);

typedef struct element_compilation_ctx element_compilation_ctx;
element_result element_create_compilation_ctx(element_interpreter_ctx* interpreter, element_compilation_ctx** output);
element_result element_delete_compilation_ctx(element_compilation_ctx** context);

element_result element_declaration_to_object(const element_declaration* declaration, element_object** output);

/// if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
element_result element_object_compile(
    const element_object* object,
    element_compilation_ctx* context,
    element_object** output);

/// if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
element_result element_object_call(
    const element_object* object,
    element_compilation_ctx* context,
    const element_object* arguments,
    unsigned int arguments_count,
    element_object** output);

/// if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
element_result element_object_index(
    const element_object* object,
    element_compilation_ctx* context,
    const char* index,
    element_object** output);

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

typedef struct element_source_information
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