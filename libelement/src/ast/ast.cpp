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
    const auto str = ast_to_string(ast, 0, ast_to_mark);
    if (str.size() > output_buffer_size)
        return ELEMENT_ERROR_INVALID_SIZE;

    sprintf(output_buffer, "%s", str.c_str());
    return ELEMENT_OK;
}

element_result element_ast_get_flags(const element_ast* ast, element_ast_flags* flags)
{
    *flags = ast->flags;
    return ELEMENT_OK;
}

element_result element_ast_get_nearest_token(const element_ast* ast, const element_token** token)
{
    *token = ast->nearest_token;
    return ELEMENT_OK;
}

element_result element_ast_get_type(const element_ast* ast, element_ast_node_type* type)
{
    assert(ast);
    assert(type);
    *type = ast->type;
    return ELEMENT_OK;
}

element_result element_ast_get_value_as_identifier(const element_ast* ast, const char** value)
{
    assert(ast);
    assert(value);
    if (!ast || !ast->has_identifier())
        return ELEMENT_ERROR_INVALID_OPERATION;
    *value = ast->identifier.c_str();
    return ELEMENT_OK;
}

element_result element_ast_get_value_as_literal(const element_ast* ast, element_value* value)
{
    assert(ast);
    assert(value);
    if (!ast || !ast->has_literal())
        return ELEMENT_ERROR_INVALID_OPERATION;
    *value = ast->literal;
    return ELEMENT_OK;
}

element_result element_ast_get_parent(const element_ast* ast, element_ast** parent)
{
    assert(ast);
    assert(parent);
    *parent = ast->parent;
    return ELEMENT_OK;
}

element_result element_ast_get_child_count(const element_ast* ast, size_t* count)
{
    assert(ast);
    assert(count);
    *count = ast->children.size();
    return ELEMENT_OK;
}

element_result element_ast_get_child(const element_ast* ast, const size_t index, element_ast** child)
{
    assert(ast);
    assert(child);
    if (index >= ast->children.size())
        return ELEMENT_ERROR_INVALID_OPERATION;
    *child = ast->children[index].get();
    return ELEMENT_OK;
}

element_result element_ast_get_root(element_ast* ast, element_ast** root)
{
    *root = ast ? ast->get_root() : nullptr;
    return ELEMENT_OK;
}

element_result element_ast_get_root_const(const element_ast* ast, const element_ast** root)
{
    *root = ast ? ast->get_root() : nullptr;
    return ELEMENT_OK;
}