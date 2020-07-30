#pragma once

//STD
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <sstream>

//SELF
#include "ast_indexes.hpp"
#include "common_internal.hpp"
#include "element/ast.h"
#include "element/token.h"
#include <cassert>

using ast_unique_ptr = std::unique_ptr<element_ast, void(*)(element_ast*)>;

element_ast* ast_new_child(element_ast* parent, element_ast_node_type type);

struct element_ast
{
    element_ast(element_ast* ast_parent)
        : parent(ast_parent)
        , type(ELEMENT_AST_NODE_NONE)
    { }

    union
    {
        element_value literal = 0;   // active for AST_NODE_LITERAL
        element_ast_flags flags; // active for all other node types
    };

    element_ast_node_type type;
    std::string identifier;
    element_ast* parent = nullptr;
    std::vector<ast_unique_ptr> children;
    const element_token* nearest_token = nullptr;

    [[nodiscard]] bool has_flag(element_ast_flags flag) const 
    {
        return (flags & flag) == flag;
    }
};

inline bool ast_node_has_identifier(const element_ast* n)
{
    return n->type == ELEMENT_AST_NODE_DECLARATION
        || n->type == ELEMENT_AST_NODE_IDENTIFIER
        || n->type == ELEMENT_AST_NODE_CALL
        || n->type == ELEMENT_AST_NODE_PORT;
}

inline bool ast_node_has_literal(const element_ast* n)
{
    return n->type == ELEMENT_AST_NODE_LITERAL;
}

//SCOPE
inline bool ast_node_in_function_scope(const element_ast* n)
{
    return (n->parent && n->parent->type == ELEMENT_AST_NODE_SCOPE &&
        n->parent->parent && n->parent->parent->type == ELEMENT_AST_NODE_FUNCTION);
}

inline bool ast_node_in_lambda_scope(const element_ast* n)
{
    return (n->parent && n->parent->type == ELEMENT_AST_NODE_SCOPE &&
    n->parent->parent && n->parent->parent->type == ELEMENT_AST_NODE_LAMBDA);
}

//STRUCT
[[nodiscard]] inline bool ast_node_struct_is_valid(const element_ast* n)
{
    return n && n->type == ELEMENT_AST_NODE_STRUCT && n->children.size() > ast_idx::function::declaration;
}

[[nodiscard]] inline bool ast_node_struct_has_body(const element_ast* n)
{
    assert(ast_node_struct_is_valid(n));
    //todo: is this even valid? I thought structs _must_ have a body, even if it's of type CONSTRAINT
    return n->children.size() > ast_idx::function::body;
}

[[nodiscard]] inline const element_ast* ast_node_struct_get_declaration(const element_ast* n)
{
    assert(ast_node_struct_is_valid(n));
    return n->children[ast_idx::function::declaration].get();
}

[[nodiscard]] inline const element_ast* ast_node_struct_get_body(const element_ast* n)
{
    assert(ast_node_struct_is_valid((n)));
    assert(ast_node_struct_has_body(n));
    return n->children[ast_idx::function::body].get();
}

//FUNCTION
[[nodiscard]] inline bool ast_node_function_is_valid(const element_ast* n)
{
    return n && n->type == ELEMENT_AST_NODE_FUNCTION &&
        n->children.size() > ast_idx::function::declaration && n->children[ast_idx::function::declaration]->type == ELEMENT_AST_NODE_DECLARATION;
}

[[nodiscard]] inline bool ast_node_function_has_body(const element_ast* n)
{
    assert(ast_node_function_is_valid(n));
    //todo: is this even valid? I thought functions _must_ have a body, even if it's of type CONSTRAINT
    return n->children.size() > ast_idx::function::body;
}

[[nodiscard]] inline const element_ast* ast_node_function_get_declaration(const element_ast* n)
{
    assert(ast_node_function_is_valid(n));
    return n->children[ast_idx::function::declaration].get();
}

[[nodiscard]] inline const element_ast* ast_node_function_get_body(const element_ast* n)
{
    assert(ast_node_function_is_valid((n)));
    assert(ast_node_function_has_body(n));
    return n->children[ast_idx::function::body].get();
}

//DECLARATION
[[nodiscard]] inline bool ast_node_declaration_is_valid(const element_ast* n)
{
    //todo: do all valid declarations have inputs?
    return n && n->type == ELEMENT_AST_NODE_DECLARATION;
}

[[nodiscard]] inline bool ast_node_declaration_has_inputs(const element_ast* n)
{
    assert(ast_node_declaration_is_valid(n));
    return n->children.size() > ast_idx::declaration::inputs;
}

[[nodiscard]] inline const element_ast* ast_node_declaration_get_inputs(const element_ast* n)
{
    assert(ast_node_declaration_is_valid(n));
    assert(ast_node_declaration_has_inputs(n));
    return n->children[ast_idx::declaration::inputs].get();
}

[[nodiscard]] inline bool ast_node_declaration_has_outputs(const element_ast* n)
{
    assert(ast_node_declaration_is_valid(n));
    return n->children.size() > ast_idx::declaration::outputs;
}

[[nodiscard]] inline const element_ast* ast_node_declaration_get_outputs(const element_ast* n)
{
    assert(ast_node_declaration_is_valid(n));
    assert(ast_node_declaration_has_outputs(n));
    return n->children[ast_idx::declaration::outputs].get();
}

//MISC
inline const element_ast* get_root_from_ast(const element_ast* ast)
{
    if (!ast)
        return nullptr;

    while (ast->parent)
    {
        ast = ast->parent;
    }

    return ast;
}

struct element_parser_ctx
{
    std::shared_ptr<element_log_ctx> logger;
    std::shared_ptr<element::source_context> src_context;
    element_tokeniser_ctx* tokeniser = nullptr;

    element_ast* root = nullptr;

    // literal ::= [-+]? [0-9]+ ('.' [0-9]*)? ([eE] [-+]? [0-9]+)?
    element_result parse_literal(size_t* tindex, element_ast* ast);
    // identifier ::= '_'? [a-zA-Z#x00F0-#xFFFF] [_a-zA-Z0-9#x00F0-#xFFFF]*
    element_result parse_identifier(size_t* tindex, element_ast* ast, bool allow_reserved_args = false, bool allow_reserved_names = false);
    // type ::= ':' identifier ('.' identifier)*
    element_result parse_typename(size_t* tindex, element_ast* ast);
    // port ::= (identifier | unidentifier) type?
    element_result parse_port(size_t* tindex, element_ast* ast);
    // portlist ::= port (',' port)*
    element_result parse_portlist(size_t* tindex, element_ast* ast);
    // exprlist ::= expression (',' expression)*
    element_result parse_exprlist(size_t* tindex, element_ast* ast);
    // call ::= (identifier | literal) ('(' exprlist ')' | '.' identifier)*
    element_result parse_call(size_t* tindex, element_ast* ast);
    // lambda ::= unidentifier '(' portlist ')' body
    element_result parse_lambda(size_t* tindex, element_ast* ast);
    // expression ::= call | lambda
    element_result parse_expression(size_t* tindex, element_ast* ast);
    // qualifier ::= 'intrinsic' | 'extern'
    element_result parse_qualifiers(size_t* tindex, element_ast_flags* flags);
    // declaration ::= identifier ('(' portlist ')')?
    // note that we also grab an optional type on the end at AST level for simplicity
    element_result parse_declaration(size_t* tindex, element_ast* ast, bool find_return_type);
    // scope ::= '{' item* '}'
    element_result parse_scope(size_t* tindex, element_ast* ast);
    element_result parse_body(size_t* tindex, element_ast* ast, bool expr_requires_semicolon);
    // function ::= qualifier* declaration type? (scope | statement | interface)
    // note qualifiers parsed further out and passed in
    element_result parse_function(size_t* tindex, element_ast* ast, element_ast_flags declflags);
    // struct ::= qualifier* 'struct' declaration (scope | interface)
    // note qualifiers parsed further out and passed in
    element_result parse_struct(size_t* tindex, element_ast* ast, element_ast_flags declflags);
    element_result parse_constraint(size_t* tindex, element_ast* ast, element_ast_flags declflags);
    // namespace ::= 'namespace' identifier scope
    element_result parse_namespace(size_t* tindex, element_ast* ast);
    // item ::= namespace | struct | function
    element_result parse_item(size_t* tindex, element_ast* ast);
    // start : /^/ (<item>)* /$/;
    element_result parse(size_t* tindex, element_ast* ast);
    element_result ast_build();

	//TODO: Move to object model as this one is already disgustingly large
    element_result validate(element_ast* ast);

private:
    element_result validate_type(element_ast* ast);
    element_result validate_portlist(element_ast* ast);
    element_result validate_struct(element_ast* ast);
    element_result validate_scope(element_ast* ast);

public:
    void log(int message_code, const std::string& message = "", const element_ast* nearest_ast = nullptr) const;
    void log(const std::string& message) const;
};
