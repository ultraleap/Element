#pragma once

//STD
#include <string>
#include <memory>
#include <functional>

//SELF
#include "common_internal.hpp"
#include "element/ast.h"
#include "element/token.h"

struct element_parser_ctx
{
public:
    std::shared_ptr<element_log_ctx> logger;
    std::shared_ptr<element::source_context> src_context;
    element_tokeniser_ctx* tokeniser = nullptr;

    element_ast* root = nullptr;

    // literal ::= [-+]? [0-9]+ ('.' [0-9]*)? ([eE] [-+]? [0-9]+)?
    element_result parse_literal(element_ast* terminal);
    // identifier ::= '_'? [a-zA-Z#x00F0-#xFFFF] [_a-zA-Z0-9#x00F0-#xFFFF]*
    element_result parse_identifier(element_ast* terminal, bool allow_reserved_args = false, bool allow_reserved_names = false);
    // type ::= ':' identifier ('.' identifier)*
    element_result parse_typename(element_ast* parent);
    // port ::= (identifier | unidentifier) type?
    element_result parse_port(element_ast* parent);
    // portlist ::= port (',' port)*
    element_result parse_portlist(element_ast* parent);
    // exprlist ::= expression (',' expression)*
    element_result parse_exprlist(element_ast* parent);
    // call ::= (identifier | literal) ('(' exprlist ')' | '.' identifier)*
    element_result parse_call(element_ast* parent);
    // lambda ::= unidentifier '(' portlist ')' body
    element_result parse_lambda(element_ast* parent);
    // expression ::= call | lambda
    element_result parse_expression(element_ast* parent);
    // qualifier ::= 'intrinsic' | 'extern'
    element_result parse_qualifiers(element_ast_flags* flags);
    // declaration ::= identifier ('(' portlist ')')?
    // note that we also grab an optional type on the end at AST level for simplicity
    element_result parse_declaration(element_ast* parent, element_ast_flags flags);
    // scope ::= '{' item* '}'
    element_result parse_scope(element_ast* parent);
    // anonymous_block ::= '{' item* '}'
    element_result parse_anonymous_block(element_ast* parent);
    element_result parse_function_body(element_ast* parent);
    // function ::= qualifier* declaration type? (scope | statement | interface)
    // note qualifiers parsed further out and passed in
    element_result parse_function(element_ast* parent, element_ast_flags declflags);
    // struct ::= qualifier* 'struct' declaration (scope | interface)
    // note qualifiers parsed further out and passed in
    element_result parse_struct(element_ast* parent, element_ast_flags declflags);
    element_result parse_constraint(element_ast* parent, element_ast_flags declflags);
    // namespace ::= 'namespace' identifier scope
    element_result parse_namespace(element_ast* parent);
    // item ::= namespace | struct | function
    element_result parse_item(element_ast* parent);
    // start : /^/ (<item>)* /$/;
    element_result parse(size_t* tindex, element_ast* input_ast);
    element_result ast_build();

    element_result next_token();
    element_ast* new_ast(element_ast* parent, element_token* token, element_ast_node_type type, element_ast_flags flags = 0);
    unsigned int token_index = 0;
    element_token* previous_token = nullptr;
    element_token* token = nullptr;
    element_ast* ast = nullptr;

    //TODO: Move to object model as this one is already disgustingly large
    element_result validate(element_ast* ast);

    element_result log(element_result message_code, const std::string& message = "", const element_ast* nearest_ast = nullptr) const;
    element_result log(const std::string& message) const;

private:
    element_result validate_type(element_ast* ast);
    element_result validate_portlist(element_ast* ast);
    element_result validate_struct(element_ast* ast);
    element_result validate_scope(element_ast* ast);
};