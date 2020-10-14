#include "element/ast.h"

//STD

//LIBS

//SELF
#include "ast/ast_internal.hpp"
#include "token_internal.hpp"

void element_ast_delete(element_ast** ast)
{
    assert(ast);
    if (!ast)
        return;

    if (*ast)
        (*ast)->clear_children();

    delete *ast;
    (*ast) = nullptr;
}

element_result element_ast_to_string(const element_ast* ast, const element_ast* ast_to_mark, char* output_buffer, int output_buffer_size)
{
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!output_buffer)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    const auto str = ast_to_string(ast, 0, ast_to_mark);
    if (str.size() > output_buffer_size)
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

    sprintf(output_buffer, "%s", str.c_str());
    return ELEMENT_OK;
}

element_result element_ast_get_flags(const element_ast* ast, element_ast_flags* flags)
{
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!flags)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    *flags = ast->flags;
    return ELEMENT_OK;
}

element_result element_ast_get_nearest_token(const element_ast* ast, const element_token** token)
{
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!token)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *token = ast->nearest_token;
    return ELEMENT_OK;
}

element_result element_ast_get_type(const element_ast* ast, element_ast_node_type* type)
{
    assert(ast);
    assert(type);

    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!type)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *type = ast->type;
    return ELEMENT_OK;
}

element_result element_ast_get_value_as_identifier(const element_ast* ast, const char** value)
{
    assert(ast);
    assert(value);

    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!value)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!ast->has_identifier())
        return ELEMENT_ERROR_API_INVALID_INPUT;

    *value = ast->identifier.c_str();
    return ELEMENT_OK;
}

element_result element_ast_get_value_as_literal(const element_ast* ast, element_value* value)
{
    assert(ast);
    assert(value);
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!value)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!ast->has_literal())
        return ELEMENT_ERROR_API_INVALID_INPUT;

    *value = ast->literal;
    return ELEMENT_OK;
}

element_result element_ast_get_parent(const element_ast* ast, element_ast** parent)
{
    assert(ast);
    assert(parent);
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!parent)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *parent = ast->parent;
    return ELEMENT_OK;
}

element_result element_ast_get_child_count(const element_ast* ast, size_t* count)
{
    assert(ast);
    assert(count);
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!count)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *count = ast->children.size();
    return ELEMENT_OK;
}

element_result element_ast_get_child(const element_ast* ast, const size_t index, element_ast** child)
{
    assert(ast);
    assert(child);
    if (!ast)
        return ELEMENT_ERROR_API_AST_IS_NULL;

    if (!child)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (index >= ast->children.size())
        return ELEMENT_ERROR_API_INVALID_INPUT;

    *child = ast->children[index].get();
    return ELEMENT_OK;
}

element_result element_ast_get_root(element_ast* ast, element_ast** root)
{
    if (!root)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *root = ast ? ast->get_root() : nullptr;
    return ELEMENT_OK;
}

element_result element_ast_get_root_const(const element_ast* ast, const element_ast** root)
{
    if (!root)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *root = ast ? ast->get_root() : nullptr;
    return ELEMENT_OK;
}