#if !defined(ELEMENT_LMNT_H)
#define ELEMENT_LMNT_H

#include "element/interpreter.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct element_lmnt_archive_ctx element_lmnt_archive_ctx;

typedef struct element_lmnt_compile_options
{
    bool dummy;
} element_lmnt_compile_options;

element_result element_lmnt_archive_init(element_lmnt_archive_ctx** context);
void element_lmnt_archive_delete(element_lmnt_archive_ctx* context);

//todo: unused
typedef struct element_function element_function;
typedef struct element_compiled_function element_compiled_function;

element_result element_lmnt_compile(
    element_interpreter_ctx* context,
    const element_compiled_function* compiled_function,
    const element_lmnt_compile_options* opts,
    element_lmnt_archive_ctx* archive);

element_result element_lmnt_archive_build(
    const element_lmnt_archive_ctx* archive,
    char* buffer, size_t* size);
#if defined(__cplusplus)
}
#endif
#endif