#if !defined(ELEMENT_INTERPRETER_OBJECT_H)
#define ELEMENT_INTERPRETER_OBJECT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/common.h"
#include "element/interpreter.h"

/**
 * @brief object type
*/
typedef struct element_object element_object;

/**
 * @brief releases memory associated with object and assigns nullptr
 *
 * @param[in,out] object        object to delete
*/
void element_object_delete(element_object** object);

/**
 * @brief compilation context
*/
typedef struct element_compilation_ctx element_compilation_ctx;

/**
 * @brief creates a compilation context from an interpreter context
 *
 * @param[in] interpreter       interpreter context
 * @param[out] output           compilation context
 *
 * @return ELEMENT_OK object compiled successfully
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter context pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
*/
element_result element_compilation_ctx_create(element_interpreter_ctx* interpreter, element_compilation_ctx** output);

/**
 * @brief releases memory associated with compilation context and assigns nullptr
 *
 * @param[out] context          compilation context
 *
 * @return ELEMENT_OK object compiled successfully
*/

element_result element_compilation_ctx_delete(element_compilation_ctx** context);

/**
 * @brief wraps a declaration in an object so that it can be assigned as a
 * shared pointer in an element object
 *
 * @param[in] declaration       declaration to wrap
 * @param[out] output           result object
 *
 * @return ELEMENT_OK object compiled successfully
 * @return ELEMENT_ERROR_API_DECLARATION_IS_NULL declaration pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
*/
element_result element_declaration_to_object(const element_declaration* declaration, element_object** output);

/**
 * @brief compiles selected object using the provided context
 *
 * if result was not OK but output object exists, call element_object_to_log_message
 * for more info or assign a log callback to the interpreter
 *
 * @param[in] object            compilation target
 * @param[in] context           compilation context
 * @param[out] output           result object
 
 * @return ELEMENT_OK object compiled successfully
 * @return ELEMENT_ERROR_API_COMPILATION_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL compilation target pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output is null
 */
element_result element_object_compile(
    const element_object* object,
    element_compilation_ctx* context,
    element_object** output);

/**
 * @brief call a callable object with provided arguments
 *
 * if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
 *
 * @param[in] object            object to call
 * @param[in] context           compilation context
 * @param[in] arguments call    arguments
 * @param[in] arguments_count   call arguments count
 * @param[out] output result    result object
 *
 * @return ELEMENT_OK called object successfully
 * @return ELEMENT_ERROR_API_COMPILATION_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT arguments pointer is null
 */
element_result element_object_call(
    const element_object* object,
    element_compilation_ctx* context,
    const element_object* arguments,
    unsigned int arguments_count,
    element_object** output);

/**
 * @brief
 *
 * if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
 *
 * @param[in] object            object to index
 * @param[in] context           compilation context
 * @param[in] index             name to index
 * @param[out] output           result object
 *
 * @return ELEMENT_OK indexed object successfully
 * @return ELEMENT_ERROR_API_COMPILATION_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT index pointer is null
 */
element_result element_object_index(
    const element_object* object,
    element_compilation_ctx* context,
    const char* index,
    element_object** output);

/**
 * @brief converts an object to an instruction tree
 *
 * @param[in] object            object to convert
 * @param[out] output           result instruction tree
 *
 * @return ELEMENT_OK object converted to an instruction tree successfully
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_SERIALISATION object cannot be serialized to an instruction tree
 */
element_result element_object_to_instruction(
    const element_object* object,
    element_instruction** output);

/**
 * @brief wrapper for an error code and message
 */
struct element_error
{
    element_result message_code;
    char* message;
};

/**
 * @brief converts an object to a log message
 *
 * object must be an error, and it must be kept alive while using the log message
 *
 * @param[in] object            object to convert, must be an error object
 * @param[out] output           result log message
 *
 * @return ELEMENT_OK object converted to log message successfully
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NOT_ERROR object is not an error
 */
element_result element_object_to_log_message(
    const element_object* object,
    element_log_message* output);

/**
 * @brief source information definition
 */
typedef struct element_source_information
{
    int line;
    int character_start;
    int character_end;

    char* filename; //must be deleted by user
    char* line_in_source; //must be deleted by user
    char* text; //must be deleted by user
} element_source_information;

/**
 * @brief converts an object to source information
 *
 * @param[in] object            object to convert
 * @param[out] output           result source information
 *
 * @return ELEMENT_OK source information retrieved successfully
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 */
element_result element_object_get_source_information(
    const element_object* object,
    element_source_information* output);

/**
 * @brief converts an object to typeof information
 *
 * @param[in] object            object to convert
 * @param[out] buffer           output buffer
 * @param[in] buffer_size       output buffer size
 *
 * @return ELEMENT_OK typeof retrieved successfully
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER buffer size is less than required size
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL buffer pointer is null
 */
element_result element_object_get_typeof(
    const element_object* object,
    char* buffer,
    int buffer_size);

#if defined(__cplusplus)
}
#endif
#endif