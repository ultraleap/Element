#if !defined(ELEMENT_INTERPRETER_OBJECT_MODEL_H)
#define ELEMENT_INTERPRETER_OBJECT_MODEL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
    
#include "element/interpreter.h"

typedef struct element_interpreter_ctx element_interpreter_ctx;
typedef struct element_declaration element_declaration;
typedef struct element_object element_object;

element_result element_object_model_compile(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_object** object);

element_result element_object_model_call(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_declaration* callable,
    element_object** object);

element_result element_object_model_index(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_declaration* indexable,
    const char* index,
    element_object** object);

    #if defined(__cplusplus)
}
#endif
#endif