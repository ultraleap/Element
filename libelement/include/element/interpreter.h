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
    //C requires that a struct or union have at least one member
    bool dummy;
} element_compiler_options;

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
void element_interpreter_set_log_callback (element_interpreter_ctx* context, void (*log_callback)(const element_log_message*));

element_result element_interpreter_clear(element_interpreter_ctx* context);

element_result element_interpreter_get_function(element_interpreter_ctx* context, const char* name, const element_function** function);

element_result element_interpreter_compile_function(
    element_interpreter_ctx* context,
    const element_function* function,
    element_compiled_function** compiled_function,
    const element_compiler_options* opts);

void element_interpreter_delete_compiled_function(element_compiled_function* compiled_function);

size_t get_outputs_size(
    element_interpreter_ctx* context,
    const element_compiled_function* compiled_function);
	
element_result element_interpreter_evaluate_function(
    element_interpreter_ctx* context,
    const element_compiled_function* fn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t outputs_count,
    const element_evaluator_options* opts);


element_result element_interpreter_test_eval(element_interpreter_ctx* context, const char* fn_name, const element_value* inputs, size_t inputs_size, element_value* outputs, size_t outputs_size);
element_result element_interpreter_get_internal_typeof(element_interpreter_ctx* context, const char* string, const char* filename, char* output_string_buffer, unsigned intoutput_string_buffer_size);
element_result element_compiled_function_get_typeof_compilation(element_compiled_function* cfn, char* string_buffer, unsigned int string_buffer_size);

#if defined(__cplusplus)
}
#endif
#endif