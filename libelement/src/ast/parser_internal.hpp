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

struct element_parser_ctx
{
public:
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
    // anonymous_block ::= '{' item* '}'
    element_result parse_anonymous_block(size_t* tindex, element_ast* ast);
    element_result parse_body(size_t* tindex, element_ast* ast);
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

    void log(element_result message_code, const std::string& message = "", const element_ast* nearest_ast = nullptr) const;
    void log(const std::string& message) const;

private:
    element_result validate_type(element_ast* ast);
    element_result validate_portlist(element_ast* ast);
    element_result validate_struct(element_ast* ast);
    element_result validate_scope(element_ast* ast);
};