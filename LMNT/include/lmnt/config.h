#ifndef LMNT_CONFIG_H
#define LMNT_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmnt/platform.h"

// Header which will be included when requiring memory-related functions
#if !defined(LMNT_MEMORY_HEADER)
#define LMNT_MEMORY_HEADER <string.h>
#endif

// Memory copy/move function
#if !defined(LMNT_MEMCPY)
#define LMNT_MEMCPY memcpy
#endif

// Value type used by the library
#if !defined(LMNT_VALUE_TYPE)
#define LMNT_VALUE_TYPE float
#endif

// Compiler definition used to mark functions and arrays as for "fast execution"
// This can be used to e.g. mark some code/data to be built in a different section
// This definition is used similarly to the following examples:
//     LMNT_ATTR_FAST void some_function();
//     LMNT_ATTR_FAST const char some_data[1024];
#if !defined(LMNT_ATTR_FAST)
#define LMNT_ATTR_FAST
#endif

// If defined and enabled, allows the interpreter to use the GNU C "computed gotos"/"labels-as-values" extension
// This allows a <TODO> performance boost
// By default this is auto-enabled for GNU C builds
#if !defined(LMNT_USE_COMPUTED_GOTOS)
#if defined(__GNUC__)
#define LMNT_USE_COMPUTED_GOTOS 1
#endif
#endif

// printf function to use for debugging output
// Never called unless requested by the user (e.g. lmnt_archive_print) or debug flags are enabled
#if !defined(LMNT_PRINTF_HEADER)
#define LMNT_PRINTF_HEADER <stdio.h>
#endif

#if !defined(LMNT_PRINTF)
#define LMNT_PRINTF printf
#endif

// sincosf function available on the target system
// this function should be available in <math.h>
// if left undefined, falls back to separate calls to sinf and cosf
// #define LMNT_SINCOSF sincosf

// Defines whether to allow entries in the constants section of the stack to be modified
// This was intended to be used to add persistent state between execution runs
// However, current versions of Element/LMNT do not make use of it
// #define LMNT_ALLOW_MODIFYING_STACK_CONSTANTS

// Prints every instruction evaluated
// This is, unsurprisingly, VERY spammy
// #define LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS


#ifdef __cplusplus
}
#endif

#endif