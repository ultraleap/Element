#if !defined(ELEMENT_INTERPRETER_OBJECT_MODEL_H)
#define ELEMENT_INTERPRETER_OBJECT_MODEL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/interpreter.h"

/// <summary>
///
/// </summary>
/// <param name="declaration"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_declaration_to_object(
    const element_declaration* declaration,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="context"></param>
/// <param name="compilable"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_compile(
    element_interpreter_ctx* context,
    const element_object* compilable,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="context"></param>
/// <param name="callable"></param>
/// <param name="arguments"></param>
/// <param name="arguments_count"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_call(
    element_interpreter_ctx* context,
    const element_object* callable,
    const element_object arguments[],
    unsigned int arguments_count,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="context"></param>
/// <param name="indexable"></param>
/// <param name="index"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_index(
    element_interpreter_ctx* context,
    const element_object* indexable,
    const char* index,
    element_object** output);

/// <summary>
///
/// </summary>
/// <param name="object"></param>
/// <param name="output"></param>
/// <returns></returns>
element_result element_object_to_instruction(
    const element_object* object,
    element_instruction** output);

/*
 * if we have typeof be an enum, then we don't need is_xyz functions for each type of thing in the object model
 * element_typeof_to_string
 * element_object_get_typeof
 * element_object_get_name
 * element_object_get_source_info
 * element_source_info
 * {
 *    char* filename
 *    char* line
 *    int column_start
 *    int length
 * }
 * element_object_is_error
 * element_object_get_error_info
 * element_error_info
 * {
 *      element_result result;
        char* message; //owning or not?
        source_info
 * }
 *
 */

#if defined(__cplusplus)
}
#endif
#endif