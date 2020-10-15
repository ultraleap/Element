#if !defined(ELEMENT_INTERPRETER_H)
#define ELEMENT_INTERPRETER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/common.h"

typedef struct element_compiler_options
{
    /* When set to true, functions being compiled must be a valid boundary function
     * 1. Type annotations for all inputs are provided, and those types are serialisable
     * 2. Type annotations for the output is provided, and that type is serialisable */
    bool check_valid_boundary_function;

    //overrides check_valid_boundary_function for nullary functions (no inputs)
    bool check_valid_boundary_function_when_nullary;
} element_compiler_options;

//passing nullptr where a element_compiler_options struct is expected will use these values
const element_compiler_options element_compiler_options_default = {
    true,
    true
};

typedef struct element_evaluator_options
{
    //C requires that a struct or union have at least one member
    bool dummy;
} element_evaluator_options;

typedef struct element_interpreter_ctx element_interpreter_ctx;

element_result element_interpreter_create(
    element_interpreter_ctx** interpreter);

void element_interpreter_delete(
    element_interpreter_ctx** interpreter);

element_result element_interpreter_load_string(
    element_interpreter_ctx* interpreter,
    const char* string, 
    const char* filename);

element_result element_interpreter_load_file(
    element_interpreter_ctx* interpreter,
    const char* file);

element_result element_interpreter_load_files(
    element_interpreter_ctx* interpreter, 
    const char** files, 
    int files_count);

element_result element_interpreter_load_package(
    element_interpreter_ctx* interpreter, 
    const char* package);

element_result element_interpreter_load_packages(
    element_interpreter_ctx* interpreter, 
    const char** packages, 
    int packages_count);

element_result element_interpreter_load_prelude(
    element_interpreter_ctx* interpreter);

element_result element_interpreter_clear(
    element_interpreter_ctx* interpreter);

element_result element_interpreter_set_log_callback(
    element_interpreter_ctx* interpreter,
    void (*log_callback)(const element_log_message*, void*),
    void* user_data);

element_result element_interpreter_set_parse_only(
    element_interpreter_ctx* interpreter, 
    bool parse_only);

typedef struct element_inputs
{
    element_value* values;
    int count;
} element_inputs;

typedef struct element_outputs
{
    element_value* values;
    int count;
} element_outputs;

typedef struct element_declaration element_declaration;

typedef struct element_instruction element_instruction;

void element_delete_declaration(
    element_declaration** declaration);

void element_delete_instruction(
    element_instruction** instruction);

element_result element_interpreter_find(
    element_interpreter_ctx* interpreter,
    const char* path,
    element_declaration** declaration);

element_result element_interpreter_compile(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_instruction** instruction);

element_result element_interpreter_evaluate(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const element_instruction* instruction,
    const element_inputs* inputs,
    element_outputs* outputs);

element_result element_interpreter_compile_expression(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const char* expression_string,
    element_instruction** instruction);

element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs);

element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    char* buffer,
    int buffer_size);

#if defined(__cplusplus)
}
#endif
#endif