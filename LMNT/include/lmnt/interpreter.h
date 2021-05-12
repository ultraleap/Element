#ifndef LMNT_INTERPRETER_H
#define LMNT_INTERPRETER_H

#include <assert.h>
#include <stdlib.h>
#include "lmnt/common.h"
#include "lmnt/archive.h"
#include "lmnt/extcalls.h"

// Declare op function signature
typedef lmnt_result(*lmnt_op_fn)(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

enum
{
    LMNT_ISTATUS_CMP_EQ = (1 << 0),  // equal
    LMNT_ISTATUS_CMP_LT = (1 << 1),  // less than
    LMNT_ISTATUS_CMP_GT = (1 << 2),  // greater than
    LMNT_ISTATUS_CMP_UN = (1 << 7),  // unordered (i.e. NaNs present)
};

// Main interpreter context struct
struct lmnt_ictx
{
    // user-provided data
    char* memory_area;
    size_t memory_area_size;
    const lmnt_extcall_info* extcalls;
    size_t extcalls_count;
    // archive
    lmnt_archive archive;
    // runtime data
    lmnt_value* stack;
    lmnt_value* writable_stack;
    size_t stack_count;
    // current def data
    const lmnt_def* cur_def;
    lmnt_loffset cur_instr;
    size_t cur_stack_count;
    const lmnt_op_fn* op_functions;
    uint32_t status_flags;
};


// Initialise an interpreter context with the specified memory area and size in bytes
// The memory space passed in must remain allocated for the lifetime of the context
// Returns: LMNT_OK or an error
lmnt_result lmnt_init(lmnt_ictx* ctx, char* mem, size_t mem_size);

// Load an archive into the specified interpreter context
// This copies the contents of data into the context's memory space
// As a result, the memory pointed to by data does not need to remain alive beyond returning from this function
// Note that essentially no validation is performed on the archive by loading it
// Returns: LMNT_OK or an error
lmnt_result lmnt_load_archive(lmnt_ictx* ctx, const char* data, size_t data_size);

// Begin loading an archive into the specified interpreter context
// Returns: LMNT_OK or an error
lmnt_result lmnt_load_archive_begin(lmnt_ictx* ctx);
// Appends data to the specified interpreter context's current archive
// Returns: LMNT_OK or an error
lmnt_result lmnt_load_archive_append(lmnt_ictx* ctx, const char* data, size_t data_size);
// Finishes loading the current archive into the specified interpreter context
// Returns: LMNT_OK or an error
lmnt_result lmnt_load_archive_end(lmnt_ictx* ctx);

// Load an archive into the specified intepreter context, using the archive data in place
// This copies the archive's constants table into the context's memory space
// The archive data passed in must remain alive for the duration this archive is loaded
// Note that essentially no validation is performed on the archive by loading it
// Returns: LMNT_OK or an error
lmnt_result lmnt_load_inplace_archive(lmnt_ictx* ctx, const char* data, size_t data_size);

// Validates the archive's contents and fills in some information only available at runtime
// The validation_result argument is optional and can be NULL
// If a validation error occurs, this function will return LMNT_ERROR_INVALID_ARCHIVE
// In this case, if a valid pointer has been passed in as validation_result, it will write a more detailed error
// Returns: LMNT_OK or an error
lmnt_result lmnt_prepare_archive(lmnt_ictx* ctx, lmnt_validation_result* validation_result);

// Convenience function for lmnt_archive_find_def
lmnt_result lmnt_find_def(const lmnt_ictx* ctx, const char* name, const lmnt_def** def);

LMNT_ATTR_FAST lmnt_result lmnt_update_args(
    lmnt_ictx* ctx, const lmnt_def* def,
    const lmnt_offset offset, const lmnt_value* args, const lmnt_offset count);

static lmnt_result lmnt_update_arg(
    lmnt_ictx* ctx, const lmnt_def* def,
    const lmnt_offset offset, const lmnt_value arg)
{
    return lmnt_update_args(ctx, def, offset, &arg, 1);
}

// Executes the specified LMNT function in the provided interpreter context
// args_count must exactly match the number of arguments expected by the LMNT function
// If the function expects no arguments, args may be NULL and args_count should be 0
// The rvals argument may be NULL if there is no need to capture the return values
// If rvals is non-null, rvals_count must be at least as large as the number of return values
// Returns: the number of return values written, or an error
LMNT_ATTR_FAST lmnt_result lmnt_execute(
    lmnt_ictx* ctx, const lmnt_def* def,
    lmnt_value* rvals, const lmnt_offset rvals_count);

// Resumes the specified LMNT function which must previously have been interrupted
// The rvals argument may be NULL if there is no need to capture the return values
// If rvals is non-null, rvals_count must be at least as large as the number of return values
// Returns: the number of return values written, or an error
LMNT_ATTR_FAST lmnt_result lmnt_resume(
    lmnt_ictx* ctx, const lmnt_def* def,
    lmnt_value* rvals, const lmnt_offset rvals_count);

// Interrupts a currently-executing LMNT function
// Note that there is currently no thread-safety mechanism in the library
// This is implemented as a single pointer write, which may be atomic on the target architecture
lmnt_result lmnt_interrupt(lmnt_ictx* ctx);

#endif
