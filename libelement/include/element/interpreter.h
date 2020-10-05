#if !defined(ELEMENT_INTERPRETER_H)
#define ELEMENT_INTERPRETER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "element/common.h"

typedef enum
{
    ELEMENT_ITEM_UNKNOWN,
    ELEMENT_ITEM_ROOT,
    ELEMENT_ITEM_STRUCT,
    ELEMENT_ITEM_CONSTRAINT,
    ELEMENT_ITEM_FUNCTION,
    ELEMENT_ITEM_NAMESPACE
} element_item_type;

typedef struct element_compiler_options
{
    /* When set to true, functions being compiled must be a valid boundary function
     * 1. Type annotations for all inputs are provided, and those types are serialisable
     * 2. Type annotations for the output is provided, and that type is serialisable
     */
    bool check_valid_boundary_function;

    //overrides check_valid_boundary_function for nullary functions (no inputs)
    bool check_valid_boundary_function_when_nullary;

    enum compiled_result_kind
    {
        //It will compile to an expression tree if possible, otherwise it will fall back to the object model
        AUTOMATIC,
        //It will compile to an expression tree, or error
        EXPRESSION_TREE_ONLY,
        //It will compile to something in the object model.
        //note: this could still be an expression tree (e.g. when using some intrinsics), but there's no guarantee (e.g. structs and functions will never be expression trees)
        OBJECT_MODEL_ONLY,
    };

    compiled_result_kind desired_result;
} element_compiler_options;

//passing nullptr where a element_compiler_options struct is expected will use these values
//todo: do we want to expose this global in the header? doesn't matter if it's modified since it's inline, but as long users can just pass nullptr... don't think we need it
//  just need to make sure we document the defaults and that the documentation doesn't get out of sync
inline element_compiler_options element_compiler_options_default = {
    true,
    false, //todo: this should not exist/be true, but our tests rely on this behaviour right now
    element_compiler_options ::compiled_result_kind::AUTOMATIC
};

typedef struct element_evaluator_options
{
    //C requires that a struct or union have at least one member
    bool dummy;
} element_evaluator_options;

typedef struct element_interpreter_ctx element_interpreter_ctx;
typedef struct element_function element_function;
typedef struct element_compiled_function element_compiled_function;

element_result element_interpreter_create(element_interpreter_ctx** context);
void element_interpreter_delete(element_interpreter_ctx* context);

element_result element_interpreter_load_string(element_interpreter_ctx* context, const char* string, const char* filename);
element_result element_interpreter_load_file(element_interpreter_ctx* context, const char* file);
element_result element_interpreter_load_files(element_interpreter_ctx* context, const char** files, int files_count);
element_result element_interpreter_load_package(element_interpreter_ctx* context, const char* package);
element_result element_interpreter_load_packages(element_interpreter_ctx* context, const char** packages, int packages_count);
element_result element_interpreter_load_prelude(element_interpreter_ctx* context);
void element_interpreter_set_log_callback(element_interpreter_ctx* context, void (*log_callback)(const element_log_message*, void*), void* user_data);
void element_interpreter_parse_only_mode(element_interpreter_ctx* context, bool parse_only);

element_result element_interpreter_clear(element_interpreter_ctx* context);

typedef struct element_declaration element_declaration;
typedef struct element_object element_object;

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

element_result element_delete_declaration(
    element_interpreter_ctx* context,
    element_declaration** declaration);

element_result element_delete_object(
    element_interpreter_ctx* context,
    element_object** object);

//call element_delete_declaration when you're done with declaration
element_result element_interpreter_find(
    element_interpreter_ctx* context,
    const char* path,
    element_declaration** declaration);

//call element_delete_object when you're done with object
element_result element_interpreter_compile(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_object** object);

element_result element_interpreter_evaluate(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const element_object* object,
    const element_inputs* inputs,
    element_outputs* outputs);

element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs);

element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const char* expression_string,
    char* buffer,
    int buffer_size);

//typedef struct element_metainfo element_metainfo;

//element_result element_metainfo_for_evaluable(
//    const element_evaluable* evaluable,
//    element_metainfo** metainfo);

//element_result element_metainfo_get_typeof(const element_metainfo* metainfo, char* buffer, int buffer_size);

#if defined(__cplusplus)
}
#endif
#endif