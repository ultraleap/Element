#include "ast/ast_internal.hpp"

//STD
#include <vector>
#include <cassert>
#include <memory>

//LIBS
#include "MemoryPool.h"

//SELF
#include "ast/ast_indexes.hpp"
#include "token_internal.hpp"
#include "configuration.hpp"
#include "log_errors.hpp"

element_ast::element_ast(element_ast* ast_parent)
    : type(ELEMENT_AST_NODE_NONE)
    , parent(ast_parent)
{
    children.reserve(20);
}

void element_ast::move(element_ast* from, element_ast* to, bool reparent)
{
    assert(from != to);
    element_ast* new_parent = reparent ? from->parent : to->parent;
    to->clear_children();

    *to = std::move(*from);
    for (auto& child : to->children)
        child->parent = to;

    to->parent = new_parent;
}

// AST memory pool
static MemoryPool<element_ast> ast_pool;

static void delete_ast_unique_ptr(element_ast* p)
{
    ast_pool.deleteElement(p);
}

element_ast* element_ast::new_child(element_ast_node_type type)
{
    // return std::make_unique<element_ast>(this);
    element_ast* node = ast_pool.newElement(this);
    node->type = type;
    node->flags = 0;
    auto child = ast_unique_ptr(node, delete_ast_unique_ptr);
    assert(child);

    child->parent = this;
    element_ast* child_raw = child.get();
    children.push_back(std::move(child));
    return child_raw;
}

void element_ast::clear_children()
{
    children.clear();
}

bool element_ast::has_flag(element_ast_flags flag) const
{
    return (flags & flag) == flag;
}

bool element_ast::has_identifier() const
{
    return type == ELEMENT_AST_NODE_DECLARATION
           || type == ELEMENT_AST_NODE_IDENTIFIER
           || type == ELEMENT_AST_NODE_CALL
           || type == ELEMENT_AST_NODE_PORT;
}

bool element_ast::has_literal() const
{
    return type == ELEMENT_AST_NODE_LITERAL;
}

bool element_ast::in_function_scope() const
{
    return (parent && parent->type == ELEMENT_AST_NODE_SCOPE && parent->parent && parent->parent->type == ELEMENT_AST_NODE_FUNCTION);
}

bool element_ast::in_lambda_scope() const
{
    return (parent && parent->type == ELEMENT_AST_NODE_SCOPE && parent->parent && parent->parent->type == ELEMENT_AST_NODE_LAMBDA);
}

bool element_ast::struct_is_valid() const
{
    return type == ELEMENT_AST_NODE_STRUCT && children.size() > ast_idx::function::declaration;
}

bool element_ast::struct_has_body() const
{
    assert(struct_is_valid());
    //todo: is this even valid? I thought structs _must_ have a body, even if it's of type CONSTRAINT
    return children.size() > ast_idx::function::body;
}

const element_ast* element_ast::struct_get_declaration() const
{
    assert(struct_is_valid());
    return children[ast_idx::function::declaration].get();
}

const element_ast* element_ast::struct_get_body() const
{
    assert(struct_is_valid());
    assert(struct_has_body());
    return children[ast_idx::function::body].get();
}

bool element_ast::function_is_valid() const
{
    return type == ELEMENT_AST_NODE_FUNCTION && children.size() > ast_idx::function::declaration && children[ast_idx::function::declaration]->type == ELEMENT_AST_NODE_DECLARATION;
}

bool element_ast::function_has_body() const
{
    assert(function_is_valid());
    //todo: is this even valid? I thought functions _must_ have a body, even if it's of type CONSTRAINT
    return children.size() > ast_idx::function::body;
}

const element_ast* element_ast::function_get_declaration() const
{
    assert(function_is_valid());
    return children[ast_idx::function::declaration].get();
}

const element_ast* element_ast::function_get_body() const
{
    assert(function_is_valid());
    assert(function_has_body());
    return children[ast_idx::function::body].get();
}

bool element_ast::constraint_is_valid() const
{
    return type == ELEMENT_AST_NODE_CONSTRAINT && children.size() > ast_idx::function::declaration;
}

const element_ast* element_ast::constraint_get_declaration() const
{
    assert(constraint_is_valid());
    return children[ast_idx::function::declaration].get();
}

bool element_ast::declaration_is_valid() const
{
    assert(children.size() > ast_idx::declaration::outputs);
    return type == ELEMENT_AST_NODE_DECLARATION;
}

bool element_ast::declaration_has_inputs() const
{
    assert(declaration_is_valid());
    return children.size() > ast_idx::declaration::inputs;
}

const element_ast* element_ast::declaration_get_inputs() const
{
    assert(declaration_is_valid());
    assert(declaration_has_inputs());
    return children[ast_idx::declaration::inputs].get();
}

bool element_ast::declaration_has_outputs() const
{
    assert(declaration_is_valid());
    return children[ast_idx::declaration::outputs]->type != ELEMENT_AST_NODE_UNSPECIFIED_TYPE;
}

const element_ast* element_ast::declaration_get_outputs() const
{
    assert(declaration_is_valid());
    return children[ast_idx::declaration::outputs].get();
}

bool element_ast::declaration_is_intrinsic() const
{
    assert(declaration_is_valid());
    return has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
}

bool element_ast::declaration_has_portlist() const
{
    assert(declaration_is_valid());
    return !children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);
}

element_ast* element_ast::get_root()
{
    element_ast* ast = this;

    while (ast->parent)
        ast = ast->parent;

    return ast;
}

const element_ast* element_ast::get_root() const
{
    const element_ast* ast = this;

    while (ast->parent)
        ast = ast->parent;

    return ast;
}