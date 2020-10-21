#if !defined(ELEMENT_AST_H)
#define ELEMENT_AST_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "element/common.h"
#include "element/token.h"

/**
 * @brief ast node type
 */
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

/**
 * @brief flags to represent additional qualifiers not encoded into the ast node type directly
 */
typedef uint32_t element_ast_flags;

/**
 * @brief flag to identify a node as having an intrinsic qualifier
 */
static const element_ast_flags ELEMENT_AST_FLAG_DECL_INTRINSIC = (1U << 1);


/**
 * @brief flag to identify a node as having no input arguments
 */
static const element_ast_flags ELEMENT_AST_FLAG_DECL_EMPTY_INPUT = (1U << 2);

/**
 * @brief opaque pointer to element_ast
 */
typedef struct element_ast element_ast;

/**
 * @brief deletes an ast node and all children
 *
 * @param[in,out] ast           ast node to be deleted, assigns nullptr on deletion
 */
void element_ast_delete(
    element_ast** ast);

/**
 * @brief  obtains any flags set on an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] flags            flags set on node
 *
 * @return ELEMENT_OK success
 */
element_result element_ast_get_flags(
    const element_ast* ast,
    element_ast_flags* flags);

/**
 * @brief obtains the nearest token from an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] token            nearest token to ast node
 *
 * @return ELEMENT_OK success
 */
element_result element_ast_get_nearest_token(
    const element_ast* ast, 
    const element_token** token);

/**
 * @brief obtains the ast node type of an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] type             ast node type of the ast node
 *
 * @return ELEMENT_OK success
 */
element_result element_ast_get_type(
    const element_ast* ast, 
    element_ast_node_type* type);

/**
 * @brief obtains the identifier of an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] value            identifier name of the ast node
 *
 * @return ELEMENT_OK obtained the identifier successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL value is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT value is not an identifier
 */
element_result element_ast_get_value_as_identifier(
    const element_ast* ast, 
    const char** value);

/**
 * @brief obtains the literal value of an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] value            literal value of the ast node
 *
 * @return ELEMENT_OK obtained the literal value successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL value pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT value is not a literal
 */
element_result element_ast_get_value_as_literal(
    const element_ast* ast, 
    element_value* value);

/**
 * @brief obtains the parent of an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] parent           parent ast node of the ast node
 *
 * @return ELEMENT_OK obtained the parent node successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL ast node has no parent
 */
element_result element_ast_get_parent(
    const element_ast* ast, 
    element_ast** parent);

/**
 * @brief counts the number of immediate children of an ast node
 *
 * @param[in] ast               ast node to check
 * @param[out] count            child count
 *
 * @return ELEMENT_OK obtained the child count successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL count pointer is null
 */
element_result element_ast_get_child_count(
    const element_ast* ast, 
    size_t* count);

/**
 * @brief obtains an immediate child of an ast node
 *
 * @param[in] ast               ast node to check
 * @param[in] index             index of the child node to retrieve
 * @param[out] child            child ast node
 *
 * @return ELEMENT_OK obtained the immediate child ast node successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL child pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT index exceeds child count
 */
element_result element_ast_get_child(
    const element_ast* ast, 
    size_t index, 
    element_ast** child);

/**
 * @brief obtains the root node of the tree from a given node
 *
 * @param[in] ast               ast node to check
 * @param[out] root             root node of the ast tree
 *
 * @return ELEMENT_OK root node retrieved successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL root node pointer is null
 */
element_result element_ast_get_root(
    element_ast* ast, 
    element_ast** root);

/**
 * @brief obtains the root node of the tree from a given node
 *
 * @param[in] ast               ast node to check
 * @param[out] root             root node of the ast tree
 *
 * @return ELEMENT_OK root node retrieved successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_OUTPUT_IS_NULL root node pointer is null
 */
element_result element_ast_get_root_const(
    const element_ast* ast, 
    const element_ast** root);

/**
 * @brief converts an ast node to string form
 *
 * @param[in] ast                   ast node to convert to string
 * @param[in] ast_to_mark           ast node to use as an indicator to highlight text, can be nullptr
 * @param[out] output_buffer        output data buffer
 * @param[in] output_buffer_size    size of the output buffer
 *
 * @return ELEMENT_OK ast node converted to string successfully
 * @return ELEMENT_ERROR_API_AST_IS_NULL ast node pointer is null
 * @return ELEMENT_ERROR_API_INVALID_INPUT output_buffer pointer is null
 * @return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER result size exceeds output_buffer_size
 */
element_result element_ast_to_string(
    const element_ast* ast, 
    const element_ast* ast_to_mark, 
    char* output_buffer, 
    int output_buffer_size);

#if defined(__cplusplus)
}
#endif
#endif