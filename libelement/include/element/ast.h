#if !defined(ELEMENT_AST_H)
#define ELEMENT_AST_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/common.h"
#include "element/token.h"

/// <summary>
///
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
///
/// </summary>
typedef uint32_t element_ast_flags;

/// <summary>
///
/// </summary>
static const element_ast_flags ELEMENT_AST_FLAG_DECL_INTRINSIC = (1U << 1);

/// <summary>
///
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
///
/// </summary>
/// <param name="ast"></param>
void element_ast_delete(
    element_ast** ast);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="flags"></param>
/// <returns></returns>
element_result element_ast_get_flags(
    const element_ast* ast,
    element_ast_flags* flags);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="token"></param>
/// <returns></returns>
element_result element_ast_get_nearest_token(
    const element_ast* ast, 
    const element_token** token);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="type"></param>
/// <returns></returns>
element_result element_ast_get_type(
    const element_ast* ast, 
    element_ast_node_type* type);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="value"></param>
/// <returns></returns>
element_result element_ast_get_value_as_identifier(
    const element_ast* ast, 
    const char** value);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="value"></param>
/// <returns></returns>
element_result element_ast_get_value_as_literal(
    const element_ast* ast, 
    element_value* value);

/// <summary>
///
/// </summary>
/// <param name="ast"></param>
/// <param name="parent"></param>
/// <returns></returns>
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