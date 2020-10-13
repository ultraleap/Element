#if !defined(ELEMENT_INTERPRETER_H)
#define ELEMENT_INTERPRETER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/common.h"

/// <summary>
///
/// </summary>
typedef struct element_compiler_options
{
    /* When set to true, functions being compiled must be a valid boundary function
     * 1. Type annotations for all inputs are provided, and those types are serialisable
     * 2. Type annotations for the output is provided, and that type is serialisable */
    bool check_valid_boundary_function;

    //overrides check_valid_boundary_function for nullary functions (no inputs)
    bool check_valid_boundary_function_when_nullary;
} element_compiler_options;

/// <summary>
///
/// </summary>
inline element_compiler_options element_compiler_options_default = 
{
    /// passing nullptr where a element_compiler_options struct is expected will use these values
    /// todo: do we want to expose this global in the header? doesn't matter if it's modified since it's inline, but as long users can just pass nullptr... don't think we need it
    /// just need to make sure we document the defaults and that the documentation doesn't get out of sync
    true,
    false, //todo: this should not exist/be true, but our tests rely on this behaviour right now
};

/// <summary>
///
/// </summary>
typedef struct element_evaluator_options
{
    //C requires that a struct or union have at least one member
    bool dummy;
} element_evaluator_options;

/// <summary>
///
/// </summary>
typedef struct element_interpreter_ctx element_interpreter_ctx;

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <returns></returns>
element_result element_interpreter_create(
    element_interpreter_ctx** interpreter);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
void element_interpreter_delete(
    element_interpreter_ctx** interpreter);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="string"></param>
/// <param name="filename"></param>
/// <returns></returns>
element_result element_interpreter_load_string(
    element_interpreter_ctx* interpreter,
    const char* string, 
    const char* filename);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="file"></param>
/// <returns></returns>
element_result element_interpreter_load_file(
    element_interpreter_ctx* interpreter,
    const char* file);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="files"></param>
/// <param name="files_count"></param>
/// <returns></returns>
element_result element_interpreter_load_files(
    element_interpreter_ctx* interpreter, 
    const char** files, 
    int files_count);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="package"></param>
/// <returns></returns>
element_result element_interpreter_load_package(
    element_interpreter_ctx* interpreter, 
    const char* package);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="packages"></param>
/// <param name="packages_count"></param>
/// <returns></returns>
element_result element_interpreter_load_packages(
    element_interpreter_ctx* interpreter, 
    const char** packages, 
    int packages_count);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <returns></returns>
element_result element_interpreter_load_prelude(
    element_interpreter_ctx* interpreter);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <returns></returns>
element_result element_interpreter_clear(
    element_interpreter_ctx* interpreter);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="log_callback"></param>
/// <param name="user_data"></param>
void element_interpreter_set_log_callback(
    element_interpreter_ctx* interpreter,
    void (*log_callback)(const element_log_message*, void*),
    void* user_data);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="parse_only"></param>
void element_interpreter_set_parse_only(
    element_interpreter_ctx* interpreter, 
    bool parse_only);

/// <summary>
///
/// </summary>
typedef struct element_declaration element_declaration;

/// <summary>
///
/// </summary>
typedef struct element_object element_object;

/// <summary>
///
/// </summary>
typedef struct element_instruction element_instruction;

/// <summary>
///
/// </summary>
typedef struct element_inputs
{
    element_value* values;
    int count;
} element_inputs;

/// <summary>
///
/// </summary>
typedef struct element_outputs
{
    element_value* values;
    int count;
} element_outputs;

/// <summary>
///
/// </summary>
/// <param name="declaration"></param>
/// <returns></returns>
element_result element_delete_declaration(
    element_declaration** declaration);

/// <summary>
///
/// </summary>
/// <param name="object"></param>
/// <returns></returns>
element_result element_delete_object(
    element_object** object);

/// <summary>
///
/// </summary>
/// <param name="instruction"></param>
/// <returns></returns>
element_result element_delete_instruction(
    element_instruction** instruction);

/// <summary>
/// call element_delete_declaration when you're done with declaration
/// </summary>
/// <param name="interpreter"></param>
/// <param name="path"></param>
/// <param name="declaration"></param>
/// <returns></returns>
element_result element_interpreter_find(
    element_interpreter_ctx* interpreter,
    const char* path,
    element_declaration** declaration);

/// <summary>
/// call element_delete_instruction when you're done with the instruction tree
/// </summary>
/// <param name="interpreter"></param>
/// <param name="options"></param>
/// <param name="declaration"></param>
/// <param name="instruction"></param>
/// <returns></returns>
element_result element_interpreter_compile(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_instruction** instruction);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="options"></param>
/// <param name="instruction"></param>
/// <param name="inputs"></param>
/// <param name="outputs"></param>
/// <returns></returns>
element_result element_interpreter_evaluate(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const element_instruction* instruction,
    const element_inputs* inputs,
    element_outputs* outputs);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="options"></param>
/// <param name="expression_string"></param>
/// <param name="instruction"></param>
/// <returns></returns>
element_result element_interpreter_compile_expression(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const char* expression_string,
    element_instruction** instruction);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="options"></param>
/// <param name="expression_string"></param>
/// <param name="outputs"></param>
/// <returns></returns>
element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs);

/// <summary>
///
/// </summary>
/// <param name="interpreter"></param>
/// <param name="options"></param>
/// <param name="expression_string"></param>
/// <param name="buffer"></param>
/// <param name="buffer_size"></param>
/// <returns></returns>
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