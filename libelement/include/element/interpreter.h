#if !defined(ELEMENT_INTERPRETER_H)
    #define ELEMENT_INTERPRETER_H

    #if defined(__cplusplus)
extern "C" {
    #endif

    #include <stddef.h>
    #include "element/common.h"

/**
 * @brief compiler options definition
 */
typedef struct element_compiler_options
{
    /* When set to true, functions being compiled must be a valid boundary function
     * 1. Type annotations for all inputs are provided, and those types are serialisable
     * 2. Type annotations for the output is provided, and that type is serialisable */
    bool check_valid_boundary_function;

    //overrides check_valid_boundary_function for nullary functions (no inputs)
    bool check_valid_boundary_function_when_nullary;
} element_compiler_options;

/**
 * @brief default compiler options
 *
 * passing nullptr where a element_compiler_options struct is expected will use these values
 */
const element_compiler_options element_compiler_options_default = {
    true,
    true
};

/**
 * @brief evaluator options definition
 */
typedef struct element_evaluator_options
{
    //C requires that a struct or union have at least one member
    bool dummy;
} element_evaluator_options;

/**
 * @brief interpreter context
 */
typedef struct element_interpreter_ctx element_interpreter_ctx;

/**
 * @brief creates an interpreter context
 *
 * @param[out] interpreter      interpreter context
 *
 * @return ELEMENT_OK created an interpreter successfully
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL interpreter pointer is null
 */
ELEMENT_API element_result element_interpreter_create(
    element_interpreter_ctx** interpreter);

/**
 * @brief deletes an interpreter context
 *
 * @param[in,out] interpreter   interpreter context
 */
ELEMENT_API void element_interpreter_delete(
    element_interpreter_ctx** interpreter);

/**
 * @brief loads an element string
 *
 * @param[in] interpreter       interpreter context
 * @param[in] string            element string
 * @param[in] filename          filename used to load element data
 *
 * @return ELEMENT_OK loaded string successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL string and/or filename pointer is null
 */
ELEMENT_API element_result element_interpreter_load_string(
    element_interpreter_ctx* interpreter,
    const char* string,
    const char* filename);

/**
 * @brief loads an element file into the interpreter context
 *
 * @param[in] interpreter       interpreter context
 * @param[in] file file         name
 *
 * @return ELEMENT_OK loaded string successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL file name pointer is null
 */
ELEMENT_API element_result element_interpreter_load_file(
    element_interpreter_ctx* interpreter,
    const char* file);

/**
 * @brief load a set of element files into the interpreter context
 *
 * @param[in] interpreter       interpreter context
 * @param[in] files             file names
 * @param[in] files_count       file count
 *
 * @return ELEMENT_OK loaded string successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT file names pointer is null
 */
ELEMENT_API element_result element_interpreter_load_files(
    element_interpreter_ctx* interpreter,
    const char** files,
    int files_count);

/**
 * @brief loads an element package from a file path
 *
 * @param[in] interpreter       interpreter context
 * @param[in] package           package path pointer
 *
 * @return ELEMENT_OK loaded package successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL package path pointer is null
 * @return ELEMENT_ERROR_DIRECTORY_NOT_FOUND directory not found
*/
ELEMENT_API element_result element_interpreter_load_package(
    element_interpreter_ctx* interpreter,
    const char* package);

/**
 * @brief loads element packages from a file paths
 *
 * @param[in] interpreter       interpreter context
 * @param[in] packages          package paths pointer
 * @param[in] packages_count    number of packages to load
 *
 * @return ELEMENT_OK loaded packages successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT package paths pointer is null
 * @return ELEMENT_ERROR_DIRECTORY_NOT_FOUND directory not found
 */
ELEMENT_API element_result element_interpreter_load_packages(
    element_interpreter_ctx* interpreter,
    const char** packages,
    int packages_count);

/**
 * @brief loads element prelude
 *
 * @param[in] interpreter       interpreter context
 *
 * @return ELEMENT_OK loaded prelude successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_PRELUDE_ALREADY_LOADED prelude already loaded
 * @return ELEMENT_ERROR_DIRECTORY_NOT_FOUND prelude cannot be found
 */
ELEMENT_API element_result element_interpreter_load_prelude(
    element_interpreter_ctx* interpreter);

/**
 * @brief clears interpreter context (CURRENTLY DOES NOTHING)
 *
 * @param[in] interpreter       interpreter context
 *
 * @return ELEMENT_OK interpreter context cleared successfully
 */
ELEMENT_API element_result element_interpreter_clear(
    element_interpreter_ctx* interpreter);

/**
 * @brief sets the callback to be used to consume error message responses
 *
 * @param[in] interpreter       interpreter context
 * @param[in] log_callback      log callback function pointer
 * @param[in] user_data         user data pointer
 *
 * @return ELEMENT_OK set log callback successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 */
ELEMENT_API element_result element_interpreter_set_log_callback(
    element_interpreter_ctx* interpreter,
    element_log_callback log_callback,
    void* user_data);

/**
 * @brief sets the parse only flag
 *
 * @param[in] interpreter       interpreter context
 * @param[in] parse_only        parse only flag
 *
 * @return ELEMENT_OK parse only flag set successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 */
ELEMENT_API element_result element_interpreter_set_parse_only(
    element_interpreter_ctx* interpreter,
    bool parse_only);

/**
 * @brief element inputs structure
 */
typedef struct element_inputs
{
    const element_value* values;
    size_t count;
} element_inputs;

/**
 * @brief element outputs structure
 */
typedef struct element_outputs
{
    element_value* values;
    size_t count;
} element_outputs;

/**
 * @brief declaration
 */
typedef struct element_declaration element_declaration;

/**
 * @brief instruction
 */
typedef struct element_instruction element_instruction;

/**
 * @brief deletes a declaration, assigns nullptr
 *
 * @param[in,out] declaration   declaration to delete
 */
ELEMENT_API void element_declaration_delete(
    element_declaration** declaration);

/**
 * @brief deletes an instruction, assigns nullptr
 *
 * @param[in] instruction       instruction to delete
 */
ELEMENT_API void element_instruction_delete(
    element_instruction** instruction);

/**
 * @brief gets the size of the instruction (number of element_value's it represents), including the size of any children
 */
ELEMENT_API element_result element_instruction_get_size(
    const element_instruction* instruction,
    size_t* size);

/**
 * @brief gets the total size of all top-level inputs used by the instruction
 */
ELEMENT_API element_result element_instruction_get_function_inputs_size(
    const element_instruction* instruction,
    size_t* size);

/**
 * @brief whether the instruction is a constant (i.e. a literal number) or not
 */
ELEMENT_API element_result element_instruction_is_constant(
    const element_instruction* instruction,
    bool* constant);

/**
 * @brief converts an instruction into a human-readable string
 *
 * @param[in] instruction       instruction to represent as a string
 * @param[out] buffer           output buffer. if a non-NULL buffer is passed and ELEMENT_OK is returned, the buffer will be null-terminated.
 * @param[in,out] buffer_size   output buffer size. this is always modified to be the size required to contain the null-terminated string.
 * @return ELEMENT_OK either buffer was NULL or buffer was not NULL and buffer_size was sufficiently large
 * @return ELEMENT_ERROR_API_INSTRUCTION_IS_NULL the instruction is NULL
 * @return ELEMENT_ERROR_API_INVALID_INPUT the buffer_size is NULL
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER the buffer_size is insufficient and a non-NULL buffer was passed
*/
ELEMENT_API element_result element_instruction_to_string(
    const element_instruction* instruction,
    char* buffer,
    size_t* buffer_size);

/**
 * @brief finds a declaration from the loaded element code
 *
 * @param[in] interpreter       interpreter context
 * @param[in] path              declaration name
 * @param[out] declaration      result
 *
 * @return ELEMENT_OK found declaration successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL declaration name is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL result object pointer is null
 * @return ELEMENT_ERROR_IDENTIFIER_NOT_FOUND path was invalid as nothing was found
 */
ELEMENT_API element_result element_interpreter_find(
    const element_interpreter_ctx* interpreter,
    const char* path,
    element_declaration** declaration);

/**
 * @brief gets the name of a declaration
 *
 * @param[in]  decl       the declaration to get the name of
 * @param[inout] buffer   the buffer to populate with the declaration's name
 * @param[inout] bufsize  the size available to write to the buffer; set to bytes written on success
 * @return ELEMENT_OK found declaration successfully
 * @return ELEMENT_ERROR_API_DECLARATION_IS_NULL declaration pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL result object pointer is null
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER buffer size is too small
 */
ELEMENT_API element_result element_declaration_get_name(
    const element_declaration* decl,
    char* buffer,
    size_t* bufsize);

/**
 * @brief gets the fully-qualified name of a declaration
 *
 * @param[in]  decl       the declaration to get the name of
 * @param[inout] buffer   the buffer to populate with the declaration's name
 * @param[inout] bufsize  the size available to write to the buffer; set to bytes written on success
 * @return ELEMENT_OK found declaration successfully
 * @return ELEMENT_ERROR_API_DECLARATION_IS_NULL declaration pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL result object pointer is null
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER buffer size is too small
 */
ELEMENT_API element_result element_declaration_get_qualified_name(
    const element_declaration* decl,
    char* buffer,
    size_t* bufsize);

/**
 * @brief converts a declaration into a code fragment that represents it
 *
 * @param[in] declaration       declaration to represent as a code fragment
 * @param[in] include_body      for functions, whether to return the whole function or just its declaration
 * @param[out] buffer           output buffer. if a non-NULL buffer is passed and ELEMENT_OK is returned, the buffer will be null-terminated.
 * @param[in,out] buffer_size   output buffer size. this is always modified to be the size required to contain the null-terminated string.
 * @return ELEMENT_OK either buffer was NULL or buffer was not NULL and buffer_size was sufficiently large
 * @return ELEMENT_ERROR_API_DECLARATION_IS_NULL the declaration is NULL
 * @return ELEMENT_ERROR_API_INVALID_INPUT the buffer_size is NULL
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER the buffer_size is insufficient and a non-NULL buffer was passed
*/
ELEMENT_API element_result element_declaration_to_code(
    const element_declaration* declaration,
    bool include_defaults,
    bool include_body,
    char* buffer,
    size_t* buffer_size);

/**
 * @brief compiles a declaration into an instruction tree
 *
 * @param[in] interpreter       interpreter context
 * @param[in] options           compilation options
 * @param[in] declaration       declaration to compile
 * @param[out] instruction      result
 *
 * @return ELEMENT_OK compiled declaration successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_DECLARATION_IS_NULL declaration pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL result object pointer is null
 */
ELEMENT_API element_result element_interpreter_compile_declaration(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_instruction** instruction);

/**
 * @brief compiles an expression to an instruction tree
 *
 * @param[in] interpreter       interpreter context
 * @param[in] options           compilation options
 * @param[in] expression_string expression to compile
 * @param[out] instruction      instruction tree result
 *
 * @return ELEMENT_OK compiled an expression to an instruction tree successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL expression_string is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL instruction is null
 */
ELEMENT_API element_result element_interpreter_compile_expression(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const char* expression_string,
    element_instruction** instruction);

typedef struct element_evaluator_ctx element_evaluator_ctx;

ELEMENT_API element_result element_evaluator_create(
    element_interpreter_ctx* interpreter,
    element_evaluator_ctx** evaluator);

ELEMENT_API element_result element_evaluator_set_options(
    element_evaluator_ctx* evaluator,
    element_evaluator_options options);

ELEMENT_API element_result element_evaluator_get_options(
    element_evaluator_ctx* evaluator,
    element_evaluator_options* options);

void element_evaluator_delete(
    element_evaluator_ctx** evaluator);

/**
 * @brief evaluates an instruction tree with the provided (boundary) inputs
 *
 * @param[in] interpreter       interpreter context
 * @param[in] evaluator         evaluator context
 * @param[in] instruction       instruction to compile
 * @param[in] inputs            inputs
 * @param[out] outputs          outputs
 *
 * @return ELEMENT_OK evaluated instruction tree successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_EVALUATOR_CTX_IS_NULL evaluator pointer is null
 * @return ELEMENT_ERROR_API_INSTRUCTION_IS_NULL instruction pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT inputs pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL outputs pointer is null
 */
ELEMENT_API element_result element_interpreter_evaluate_instruction(
    element_interpreter_ctx* interpreter,
    element_evaluator_ctx* evaluator,
    const element_instruction* instruction,
    const element_inputs* inputs,
    element_outputs* outputs);

/**
 * @brief evaluates an expression
 *
 * @param[in] interpreter       interpreter context
 * @param[in] evaluator         evaluator context
 * @param[in] expression_string expression to compile
 * @param[out] outputs          outputs
 *
 * @return ELEMENT_OK evaluated an expression successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_EVALUATOR_CTX_IS_NULL evaluator pointer is null
 * @return ELEMENT_ERROR_API_INSTRUCTION_IS_NULL instruction pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT outputs pointer is null
 */
ELEMENT_API element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* interpreter,
    element_evaluator_ctx* evaluator,
    const char* expression_string,
    element_outputs* outputs);

/**
 * @brief evaluates a call_expression
 *
 * @param[in] interpreter       interpreter context
 * @param[in] evaluator         evaluator context
 * @param[in] call_expression   expression to compile
 * @param[out] outputs          outputs
 *
 * @return ELEMENT_OK evaluated an expression successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_EVALUATOR_CTX_IS_NULL evaluator pointer is null
 * @return ELEMENT_ERROR_API_INSTRUCTION_IS_NULL instruction pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT outputs pointer is null
 */
ELEMENT_API element_result element_interpreter_evaluate_call_expression(
    element_interpreter_ctx* interpreter,
    element_evaluator_ctx* evaluator,
    const char* call_expression,
    element_outputs* outputs);

/**
 * @brief determines typeof information for a given expression
 *
 * @param[in] interpreter       interpreter context
 * @param[in] expression_string expression to evaluate to discover type
 * @param[out] buffer           output buffer
 * @param[in] buffer_size       output buffer size
 *
 * @return ELEMENT_OK retrieved typeof information successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter pointer is null
 * @return ELEMENT_ERROR_API_STRING_IS_NULL expression string pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL buffer pointer is null
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER buffer size is too small
 */
ELEMENT_API element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* interpreter,
    const char* expression_string,
    char* buffer,
    int buffer_size);


ELEMENT_API element_result element_interpreter_export_lmnt(
    element_interpreter_ctx* context,
    const element_declaration** decls,
    const char** funcnames,
    size_t decls_count,
    char* buffer,
    size_t* bufsize);


    #if defined(__cplusplus)
}
    #endif
#endif