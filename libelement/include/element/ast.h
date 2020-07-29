#if !defined(ELEMENT_AST_H)
#define ELEMENT_AST_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#include "element/common.h"
#include "element/token.h"

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
    ELEMENT_AST_NODE_UNSPECIFIED_TYPE
} element_ast_node_type;

typedef uint32_t element_ast_flags;
// declaration flags
static const element_ast_flags ELEMENT_AST_FLAG_DECL_INTRINSIC = (1U << 1);
static const element_ast_flags ELEMENT_AST_FLAG_DECL_EMPTY_INPUT = (1U << 2);
static const element_ast_flags ELEMENT_AST_FLAG_DECL_IMPLICIT_RETURN = (1U << 3);

typedef struct element_ast_node element_ast_node;
typedef struct element_ast element_ast;

typedef struct element_parser_ctx element_parser_ctx;

element_result element_ast_get_type(const element_ast* node, element_ast_node_type* type);
element_result element_ast_get_value_as_identifier(const element_ast* node, const char** value);
element_result element_ast_get_value_as_literal(const element_ast* node, element_value* value);
element_result element_ast_get_parent(const element_ast* ast, element_ast** parent);
element_result element_ast_get_child_count(const element_ast* ast, size_t* count);
element_result element_ast_get_child(const element_ast* ast, const size_t index, element_ast** child);

element_result element_ast_build(element_tokeniser_ctx* tctx, element_ast** ast);
void element_ast_delete(element_ast* ast);
element_ast* ast_new_child(element_ast* parent, element_ast_node_type type);

#if defined(__cplusplus)
}
#endif
#endif