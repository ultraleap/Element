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
    bool dummy;
} element_compiler_options;

typedef struct element_evaluator_options
{
    bool dummy;
} element_evaluator_options;


typedef struct element_interpreter_ctx element_interpreter_ctx;
typedef struct element_function element_function;
typedef struct element_compiled_function element_compiled_function;

element_result element_interpreter_create(element_interpreter_ctx** ctx);
void element_interpreter_delete(element_interpreter_ctx* ctx);

element_result element_interpreter_load_string(element_interpreter_ctx* ctx, const char* string, const char* filename);
element_result element_interpreter_load_file(element_interpreter_ctx* ctx, const char* file);
element_result element_interpreter_load_files(element_interpreter_ctx* ctx, const char** files, int files_count);
element_result element_interpreter_load_package(element_interpreter_ctx* ctx, const char* package);
element_result element_interpreter_load_packages(element_interpreter_ctx* ctx, const char** packages, int packages_count);
element_result element_interpreter_load_prelude(element_interpreter_ctx* ctx);
element_result element_interpreter_print_ast(element_interpreter_ctx* ctx, const char* name);

element_result element_interpreter_clear(element_interpreter_ctx* ctx);

element_result element_interpreter_get_function(element_interpreter_ctx* ctx, const char* name, const element_function** fn);

element_result element_interpreter_compile_function(
    element_interpreter_ctx* ctx,
    const element_function* fn,
    element_compiled_function** cfn,
    const element_compiler_options* opts);

void element_interpreter_delete_compiled_function(element_compiled_function* cfn);

element_result element_interpreter_evaluate_function(
    element_interpreter_ctx* ctx,
    const element_compiled_function* fn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t outputs_count,
    const element_evaluator_options* opts);


element_result element_interpreter_test_eval(element_interpreter_ctx* ctx, const char* fn_name, const element_value* inputs, size_t inputs_size, element_value* outputs, size_t outputs_size);

#if defined(__cplusplus)
}
#endif
#endif