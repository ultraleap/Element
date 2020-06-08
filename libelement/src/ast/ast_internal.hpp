#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <numeric>
#include <sstream>



#include "ast_indexes.hpp"
#include "common_internal.hpp"
#include "configuration.hpp"
#include "element/ast.h"
#include "element/token.h"
#include <cassert>

using ast_unique_ptr = std::unique_ptr<element_ast, void(*)(element_ast*)>;

//class ast_visitor
//{
//public:
//    virtual void visit_none(element_ast* ast) = 0;
//    virtual void visit_root(element_ast* ast) = 0;
//    virtual void visit_function(element_ast* ast) = 0;
//    virtual void visit_struct(element_ast* ast) = 0;
//    virtual void visit_namespace(element_ast* ast) = 0;
//    virtual void visit_declaration(element_ast* ast) = 0;
//    virtual void visit_scope(element_ast* ast) = 0;
//    virtual void visit_constraint(element_ast* ast) = 0;
//    virtual void visit_expression(element_ast* ast) = 0;
//    virtual void visit_expression_list(element_ast* ast) = 0;
//    virtual void visit_port_list(element_ast* ast) = 0;
//    virtual void visit_port(element_ast* ast) = 0;
//    virtual void visit_typename(element_ast* ast) = 0;
//    virtual void visit_call(element_ast* ast) = 0;
//    virtual void visit_lambda(element_ast* ast) = 0;
//    virtual void visit_identifier(element_ast* ast) = 0;
//    virtual void visit_literal(element_ast* ast) = 0;
//    virtual void visit_unspecified_type(element_ast* ast) = 0;
//};

//PRINTCASE(ELEMENT_TOK_NONE);
struct element_ast
{
    element_ast_node_type type;
    std::string identifier;
    union {
        element_value literal = 0;   // active for AST_NODE_LITERAL
        element_ast_flags flags; // active for all other node types
    };
    element_ast* parent = nullptr;
    std::vector<ast_unique_ptr> children;
    const element_token* nearest_token = nullptr;

    bool has_flag(element_ast_flags flag) const 
    {
        return (flags & flag) == flag;
    }

//	//use for ast_node_as_code and ast_print at some point
//    void visit(ast_visitor& visitor)
//    {
//        visit_node(visitor , this);
//    	
//        for(auto& child : children)
//        {
//            child->visit(visitor);
//        }
//    }
//	
//private:
//    static void visit_node(ast_visitor& visitor, element_ast* ast)
//    {
//        switch(ast->type)
//        {
//        case ELEMENT_AST_NODE_NONE: visitor.visit_none(ast); break;
//        case ELEMENT_AST_NODE_ROOT: visitor.visit_root(ast); break;
//        case ELEMENT_AST_NODE_FUNCTION: visitor.visit_function(ast); break;
//        case ELEMENT_AST_NODE_STRUCT: visitor.visit_struct(ast); break;
//        case ELEMENT_AST_NODE_NAMESPACE: visitor.visit_namespace(ast); break;
//        case ELEMENT_AST_NODE_DECLARATION: visitor.visit_declaration(ast); break;
//        case ELEMENT_AST_NODE_SCOPE: visitor.visit_scope(ast); break;
//        case ELEMENT_AST_NODE_CONSTRAINT: visitor.visit_constraint(ast); break;
//        case ELEMENT_AST_NODE_EXPRESSION: visitor.visit_expression(ast); break;
//        case ELEMENT_AST_NODE_EXPRLIST: visitor.visit_expression_list(ast); break;
//        case ELEMENT_AST_NODE_PORTLIST: visitor.visit_port_list(ast); break;
//        case ELEMENT_AST_NODE_PORT: visitor.visit_port(ast); break;
//        case ELEMENT_AST_NODE_TYPENAME: visitor.visit_typename(ast); break;
//        case ELEMENT_AST_NODE_CALL: visitor.visit_call(ast); break;
//        case ELEMENT_AST_NODE_LAMBDA: visitor.visit_lambda(ast); break;
//        case ELEMENT_AST_NODE_IDENTIFIER: visitor.visit_identifier(ast); break;
//        case ELEMENT_AST_NODE_LITERAL: visitor.visit_literal(ast); break;
//        case ELEMENT_AST_NODE_UNSPECIFIED_TYPE: visitor.visit_unspecified_type(ast); break;
//	    }
//    }

#ifndef NDEBUG
    std::string ast_node_as_code;

    void ast_node_to_code();
#endif

private:

public:
    //element_ast* find_child(std::function<bool(const element_ast* elem)> function)
    //{
    //    for (const auto& t : children) {
    //        if (function(t.get()))
    //            return t.get();
    //    }
    //    return nullptr;
    //}

    //element_ast* first_child_of_type(element_ast_node_type type) const
    //{
    //    for (const auto& t : children) {
    //        if (t->type == type)
    //            return t.get();
    //    }
    //    return nullptr;
    //}

    //template <size_t N>
    //element_ast* nth_parent()
    //{
    //    element_ast* p = this;
    //    for (size_t i = 0; i < N; ++i) {
    //        if (!p) break;
    //        p = p->parent;
    //    }
    //    return p;
    //}

    //template <size_t N>
    //const element_ast* nth_parent() const
    //{
    //    const element_ast* p = this;
    //    for (size_t i = 0; i < N; ++i) {
    //        if (!p) break;
    //        p = p->parent;
    //    }
    //    return p;
    //}

    enum class walk_step { stop, step_in, next, step_out };
    using walker = std::function<walk_step(element_ast*)>;
    using const_walker = std::function<walk_step(const element_ast*)>;

    walk_step walk(const walker& fn);
    walk_step walk(const const_walker& fn) const;

    element_ast(element_ast* iparent) : parent(iparent) {}
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
    std::shared_ptr<element_log_ctx> logger = nullptr;
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

	//TODO: Move to another class as this one is already disgustingly large
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
