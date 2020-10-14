#if !defined(ELEMENT_AST_H)
#define ELEMENT_AST_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/common.h"
#include "element/token.h"

/// <summary>
/// ast node type
/// </summary>
typedef enum
{
    ELEMENT_AST_NODE_NONE = 0,
    ELEMENT_AST_NODE_ROOT,
    ELEMENT_AST_NODE_FUNCTION,
    ELEMENT_AST_NODE_STRUCT,
    ELEMENT_AST_NODE_NAMESPACE,
    ELEMENT_AST_NODE_DECLARATION,
    ELEMENT_AST_NODE_SCOPE,
    ELEMENT_AST_NODE_CONSTRAINT,
    ELEMENT_AST_NODE_EXPRESSION,
    ELEMENT_AST_NODE_EXPRLIST,
    ELEMENT_AST_NODE_PORTLIST,
    ELEMENT_AST_NODE_PORT,
    ELEMENT_AST_NODE_TYPENAME,
    ELEMENT_AST_NODE_CALL,
    ELEMENT_AST_NODE_LAMBDA,
    ELEMENT_AST_NODE_IDENTIFIER,
    ELEMENT_AST_NODE_LITERAL,
    ELEMENT_AST_NODE_NO_BODY,
    ELEMENT_AST_NODE_UNSPECIFIED_TYPE,
    ELEMENT_AST_NODE_UNSPECIFIED_DEFAULT,
    ELEMENT_AST_NODE_ANONYMOUS_BLOCK,
} element_ast_node_type;

/// <summary>
/// flags to represent additional qualifiers not encoded into the ast node type directly
/// </summary>
typedef uint32_t element_ast_flags;

/// <summary>
/// flag to identify a node as having an intrinsic qualifier
/// </summary>
static const element_ast_flags ELEMENT_AST_FLAG_DECL_INTRINSIC = (1U << 1);

/// <summary>
/// flag to identify a node as having no input arguments
/// </summary>
static const element_ast_flags ELEMENT_AST_FLAG_DECL_EMPTY_INPUT = (1U << 2);

/// <summary>
/// 
/// </summary>
typedef struct element_ast_node element_ast_node;

/// <summary>
///
/// </summary>
typedef struct element_ast element_ast;

/// <summary>
/// deletes an ast node and all children
/// </summary>
/// <param name="ast">input/output: ast node to be deleted, assigns nullptr on deletion</param>
void element_ast_delete(
    element_ast** ast);

/// <summary>
/// obtains any flags set on an ast node
/// </summary>
/// <param name="ast">input: ast node to check</param>
/// <param name="flags">output: flags set on node</param>
/// <returns>ELEMENT_OK</returns>
element_result element_ast_get_flags(
    const element_ast* ast,
    element_ast_flags* flags);

/// <summary>
/// obtains the nearest token from an ast node
/// </summary>
/// <param name="ast">input: ast node to check</param>
/// <param name="token">output: nearest token to ast node</param>
/// <returns>ELEMENT_OK</returns>
element_result element_ast_get_nearest_token(
    const element_ast* ast, 
    const element_token** token);

/// <summary>
/// obtains the ast node type of an ast node
/// </summary>
/// <param name="ast">input: ast node to check</param>
/// <param name="type">output: ast node type of the ast node</param>
/// <returns>ELEMENT_OK</returns>
element_result element_ast_get_type(
    const element_ast* ast, 
    element_ast_node_type* type);

/// <summary>
/// obtains the identifier of an ast node
/// </summary>
/// <param name="ast">input: ast node to check</param>
/// <param name="value">output: identifier name of the ast node</param>
/// <returns>
/// failure: ELEMENT_ERROR_INVALID_OPERATION if ast node is null or has no identifier
/// success: ELEMENT_OK
/// </returns>
element_result element_ast_get_value_as_identifier(
    const element_ast* ast, 
    const char** value);

/// <summary>
/// obtains the literal value of an ast node
/// </summary>
/// <param name="ast">input: ast node to check</param>
/// <param name="value">output: identifier name of the ast node</param>
/// <returns>
/// failure: ELEMENT_ERROR_INVALID_OPERATION if ast node is null or has no literal value
/// success: ELEMENT_OK
/// </returns>
element_result element_ast_get_value_as_literal(
    const element_ast* ast, 
    element_value* value);

/*******************************************************************************
 * obtains the parent of an ast node
 * @param[in] ast ast node to check
 * @param[out] parent parent ast node of the ast node
 * @retval ELEMENT_ERROR_API_AST_IS_NULL ast node is null
 * @retval ELEMENT_ERROR_API_OUTPUT_IS_NULL ast node has no parent
 * @retval ELEMENT_OK success
 ******************************************************************************/
element_result element_ast_get_parent(
    const element_ast* ast, 
    element_ast** parent);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="count"></param>
/// <returns></returns>
element_result element_ast_get_child_count(
    const element_ast* ast, 
    size_t* count);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="index"></param>
/// <param name="child"></param>
/// <returns></returns>
element_result element_ast_get_child(
    const element_ast* ast, size_t index, 
    element_ast** child);

/// <summary>
/// may return ast if it is the root
/// </summary>
/// <param name="ast"></param>
/// <param name="root"></param>
/// <returns></returns>
element_result element_ast_get_root(
    element_ast* ast, 
    element_ast** root);

/// <summary>
/// may return ast if it is the root
/// </summary>
/// <param name="ast"></param>
/// <param name="root"></param>
/// <returns></returns>
element_result element_ast_get_root_const(
    const element_ast* ast, 
    const element_ast** root);

/// <summary>
/// ast_to_mark will output "HERE" beside it, pass null if unwanted
/// </summary>
/// <param name="ast"></param>
/// <param name="ast_to_mark"></param>
/// <param name="output_buffer"></param>
/// <param name="output_buffer_size"></param>
/// <returns></returns>
element_result element_ast_to_string(
    const element_ast* ast, 
    const element_ast* ast_to_mark, 
    char* output_buffer, 
    int output_buffer_size);

#if defined(__cplusplus)
}
#endif
#endif