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

element_result element_lmnt_archive_init(element_lmnt_archive_ctx** ctx);
void element_lmnt_archive_delete(element_lmnt_archive_ctx* ctx);

element_result element_lmnt_compile(
    element_interpreter_ctx* ctx,
    const element_compiled_function* cfn,
    const element_lmnt_compile_options* opts,
    element_lmnt_archive_ctx* archive
);

element_result element_lmnt_archive_build(
    const element_lmnt_archive_ctx* archive,
    char* buffer, size_t* size
);

#if defined(__cplusplus)
}
#endif
#endif