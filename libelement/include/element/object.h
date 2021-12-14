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
 * @brief represents a parameter or return;
*/
typedef struct element_port element_port;

/**
 * @brief represents multiple ports;
*/
typedef struct element_ports element_ports;

/**
 * @brief releases memory associated with object and assigns nullptr
 *
 * @param[in,out] object        object to delete
*/
void element_object_delete(element_object** object);

/**
 * @brief object-model context
*/
typedef struct element_object_model_ctx element_object_model_ctx;

/**
 * @brief creates an object-model context from an interpreter context
 *
 * @param[in] interpreter       interpreter context
 * @param[out] output           object-model context
 *
 * @return ELEMENT_OK successfully created context
 * @return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL interpreter context pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
*/
element_result element_object_model_ctx_create(element_interpreter_ctx* interpreter, element_object_model_ctx** output);

/**
 * @brief releases memory associated with object-model context and assigns nullptr
 *
 * @param[out] context          object-model context
 *
 * @return ELEMENT_OK context deleted successfully
*/

void element_object_model_ctx_delete(element_object_model_ctx** context);

/**
 * @brief wraps a declaration in an object so that it can be assigned as a
 * shared pointer in an element object
 *
 * @param[in] declaration       declaration to wrap
 * @param[out] output           result object
 *
 * @return ELEMENT_OK object created successfully
 * @return ELEMENT_ERROR_API_DECLARATION_IS_NULL declaration pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
*/
element_result element_declaration_to_object(const element_declaration* declaration, element_object** output);

/**
 * @brief simplify selected object using the provided context
 *
 * if result was not OK but output object exists, call element_object_to_log_message
 * for more info or assign a log callback to the interpreter
 *
 * @param[in] object            unsimplified object
 * @param[in] context           object-model context
 * @param[out] output           simplified object
 
 * @return ELEMENT_OK object simplified successfully
 * @return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL unsimplified object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output is null
 */
element_result element_object_simplify(
    const element_object* object,
    element_object_model_ctx* context,
    element_object** output);

/**
 * @brief call a callable object with provided arguments
 *
 * if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
 *
 * @param[in] object            object to call
 * @param[in] context           object-model context
 * @param[in] arguments call    arguments
 * @param[in] arguments_count   call arguments count
 * @param[out] output result    result object
 *
 * @return ELEMENT_OK called object successfully
 * @return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT arguments pointer is null
 */
element_result element_object_call(
    const element_object* object,
    element_object_model_ctx* context,
    element_object** arguments,
    unsigned int arguments_count,
    element_object** output);

/**
 * @brief call a callable object with placeholder arguments
 *
 * if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
 *
 * @param[in] object            object to call
 * @param[in] context           object-model context
 * @param[out] output result    result object
 *
 * @return ELEMENT_OK called object successfully
 * @return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 */
element_result element_object_call_with_placeholders(
    const element_object* object,
    element_object_model_ctx* context,
    element_object** output);

/**
 * @brief indexes the provided object
 *
 * if result was not OK but output object exists, call element_object_to_log_message for more info or assign a log callback to the interpreter
 *
 * @param[in] object            object to index
 * @param[in] context           object-model context
 * @param[in] index             name to index
 * @param[out] output           result object
 *
 * @return ELEMENT_OK indexed object successfully
 * @return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT index pointer is null
 */
element_result element_object_index(
    const element_object* object,
    element_object_model_ctx* context,
    const char* index,
    element_object** output);

/**
 * @brief converts an object to an instruction tree
 *
 * @param[in] object            object to convert
 * @param[in] context           object-model context 
 * @param[out] output           result instruction tree
 *
 * @return ELEMENT_OK object converted to an instruction tree successfully
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL object pointer is null
 * @return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL context pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL output pointer is null
 * @return ELEMENT_ERROR_SERIALISATION object cannot be serialized to an instruction tree
 */
element_result element_object_to_instruction(
    const element_object* object,
    element_object_model_ctx* context,
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

    char* filename;       //must be deleted by user
    char* line_in_source; //must be deleted by user
    char* text;           //must be deleted by user
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
 * @brief gets the name of an object, if it has one. e.g the name of a function or struct
 */
element_result element_object_get_name(
    const element_object* object,
    char* buffer,
    size_t buffer_size);

/**
 * @brief converts an object to typeof information. e.g. ExpressionBodied for a function
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

/**
 * @brief converts an object into a code fragment that represents it
 *
 * @param[in] object       object to represent as a code fragment
 * @param[out] buffer           output buffer. if a non-NULL buffer is passed and ELEMENT_OK is returned, the buffer will be null-terminated.
 * @param[in,out] buffer_size   output buffer size. this is always modified to be the size required to contain the null-terminated string.
 * @return ELEMENT_OK either buffer was NULL or buffer was not NULL and buffer_size was sufficiently large
 * @return ELEMENT_ERROR_API_OBJECT_IS_NULL the object is NULL
 * @return ELEMENT_ERROR_API_INVALID_INPUT the buffer_size is NULL
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER the buffer_size is insufficient and a non-NULL buffer was passed
*/
element_result element_object_to_code(
    const element_object* object,
    char* buffer,
    size_t* buffer_size);

/**
 * @brief gets any and all inputs of an object, such as the parameters of a function
 *
 * The lifetime of the returned ports object is the same as that of the object it is retrieved from.
 */
element_result element_object_get_inputs(
    const element_object* object,
    const element_ports** inputs);

/**
 * @brief gets the output of an object, such as the return of a function
 *
 * The lifetime of the returned port is the same as that of the object it is retrieved from.
 */
element_result element_object_get_output(
    const element_object* object,
    const element_port** output);

/**
 * @brief gets a port from a list of ports
 *
 * The lifetime of the returned port is the same as that of the ports structure it is retrieved from.
 */
element_result element_ports_get_port(
    const element_ports* ports,
    size_t index,
    const element_port** port);

/**
 * @brief gets the number of ports in the list
 */
element_result element_ports_get_count(
    const element_ports* ports,
    size_t* count);

/**
 * @brief gets the name of the port if it has one (such as the parameter name), otherwise empty string
 */
element_result element_port_get_name(
    const element_port* port,
    const char** name);

/**
 * @brief gets the string in source of the type, e.g. func(a:MyNamespace.MyStruct) returns "MyNamespace.MyStruct"
 */
element_result element_port_get_constraint_annotation(
    const element_port* port,
    const char** annotation);

/**
 * @brief gets the constraint after interpreting the annotation, e,g. func(a:MyNamespace.MyStruct) returns an object that is the "MyStruct" struct
 */
element_result element_port_get_constraint_object(
    const element_port* port,
    element_object_model_ctx* object_model_context,
    element_object** object);

/**
 * @brief gets the default after interpreting, e,g. func(a:Num = 5.add(2)) returns an object that is a "Num" with the value "7"
 */
element_result element_port_get_default_object(
    const element_port* port,
    element_object_model_ctx* object_model_context,
    element_object** object);

    #if defined(__cplusplus)
}
    #endif
#endif