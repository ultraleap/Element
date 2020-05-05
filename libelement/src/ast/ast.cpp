#include "element/ast.h"
#include "element/token.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <memory>
#include <unordered_set>

#include <fmt/format.h>

#include "ast/ast_internal.hpp"
#include "ast/ast_indexes.hpp"
#include "token_internal.hpp"
#include "MemoryPool.h"

static std::unordered_set<std::string> qualifiers {"intrinsic"};
static std::unordered_set<std::string> constructs{ "struct", "namespace", "constraint"};
static std::unordered_set<std::string> reserved_args {};

static element_result check_reserved_words(const std::string& text, bool allow_reserved_arg)
{
    return (qualifiers.count(text) == 0 && constructs.count(text) == 0 && (allow_reserved_arg || reserved_args.count(text) == 0))
        ? ELEMENT_OK
        : ELEMENT_ERROR_INVALID_ARCHIVE; // TODO: errcode
}

//
// Token helpers
//
#define GET_TOKEN(tctx, tindex, tok) ELEMENT_OK_OR_RETURN(element_tokeniser_get_token((tctx), (tindex), &(tok)))
#define GET_TOKEN_COUNT(tctx, tcount) ELEMENT_OK_OR_RETURN(element_tokeniser_get_token_count((tctx), &(tcount)))

static int tokenlist_advance(element_tokeniser_ctx* tctx, size_t* tindex)
{
    size_t tcount;
    ELEMENT_OK_OR_RETURN(element_tokeniser_get_token_count(tctx, &tcount));

    ++(*tindex);
    if (*tindex >= tcount)
        return false;

    // TODO: do something with these, we might need them later to preserve formatting...
    const element_token* tok;
    GET_TOKEN(tctx, *tindex, tok);
    while (*tindex < tcount && tok->type == ELEMENT_TOK_NONE) {
        ++(*tindex);
        GET_TOKEN(tctx, *tindex, tok);
    }
    return (*tindex < tcount);
}

#define TOKENLIST_ADVANCE_AND_UPDATE(tctx, tindex, tok) \
{ tokenlist_advance((tctx), (tindex)); ELEMENT_OK_OR_RETURN(element_tokeniser_get_token((tctx), *(tindex), &(tok))); }

//
// AST memory pool
//
static MemoryPool<element_ast> ast_pool;

static void delete_ast_unique_ptr(element_ast* p)
{
    ast_pool.deleteElement(p);
}

static ast_unique_ptr ast_new(element_ast* parent, element_ast_node_type type = ELEMENT_AST_NODE_NONE)
{
    // return std::make_unique<element_ast>(parent);
    element_ast* node = ast_pool.newElement(parent);
    node->type = type;
    return ast_unique_ptr(node, delete_ast_unique_ptr);
}


element_result element_ast_get_type(const element_ast* node, element_ast_node_type* type)
{
    assert(node);
    assert(type);
    *type = node->type;
    return ELEMENT_OK;
}

element_result element_ast_get_value_as_identifier(const element_ast* node, const char** value)
{
    assert(node);
    assert(value);
    if (!ast_node_has_identifier(node))
        return ELEMENT_ERROR_INVALID_OPERATION;
    *value = node->identifier.c_str();
    return ELEMENT_OK;
}

element_result element_ast_get_value_as_literal(const element_ast* node, element_value* value)
{
    assert(node);
    assert(value);
    if (!ast_node_has_literal(node))
        return ELEMENT_ERROR_INVALID_OPERATION;
    *value = node->literal;
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


static void ast_clear(element_ast* n)
{
    assert(n);
    n->children.clear();
}

static void ast_move(element_ast* from, element_ast* to, bool reparent)
{
    assert(from != to);
    element_ast* new_parent = reparent ? from->parent : to->parent;
    ast_clear(to);

    *to = std::move(*from);
    to->parent = new_parent;
}

static element_ast* ast_add_child(element_ast* parent, ast_unique_ptr child)
{
    assert(parent);
    assert(child);
    assert(child->parent == parent || child->parent == NULL);
    child->parent = parent;
    element_ast* childr = child.get();
    parent->children.push_back(std::move(child));
    return childr;
}

static element_ast* ast_new_child(element_ast* parent, element_ast_node_type type = ELEMENT_AST_NODE_NONE)
{
    auto c = ast_new(parent, type);
    element_ast* cr = c.get();
    ast_add_child(parent, std::move(c));
    return cr;
}

element_result element_parser_ctx::parse_literal(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    assert(token->type == ELEMENT_TOK_NUMBER);
    ast->type = ELEMENT_AST_NODE_LITERAL;
    ast->literal = std::stof(tokeniser->text(token));
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_identifier(size_t* tindex, element_ast* ast, bool allow_reserved_args)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    assert(token->type == ELEMENT_TOK_IDENTIFIER);
    ast->nearest_token = token;
    ast->identifier.assign(tokeniser->text(token));
    tokenlist_advance(tokeniser, tindex);

    auto result = check_reserved_words(ast->identifier, allow_reserved_args);
    if (result != ELEMENT_OK) {
        log(ELEMENT_ERROR_INVALID_IDENTIFIER,
            fmt::format("Invalid identifier '{}'", ast->identifier));
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    return result;
}

element_result element_parser_ctx::parse_typename(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    assert(tok->type == ELEMENT_TOK_IDENTIFIER);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_TYPENAME;

    element_result result = ELEMENT_OK;

    while (tok->type == ELEMENT_TOK_IDENTIFIER) {
        element_ast* child = ast_new_child(ast);
        child->type = ELEMENT_AST_NODE_IDENTIFIER;

        const auto identifier_result = parse_identifier(tindex, child);
        if (identifier_result != ELEMENT_OK)
        {
            if (result == ELEMENT_OK)
                result = identifier_result;
        }

        GET_TOKEN(tokeniser, *tindex, tok);
        child->nearest_token = tok;
        if (tok->type == ELEMENT_TOK_DOT)
            TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    }

    return result;
}

element_result element_parser_ctx::parse_port(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    assert(tok->type == ELEMENT_TOK_IDENTIFIER || tok->type == ELEMENT_TOK_UNDERSCORE);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_PORT;
    if (tok->type == ELEMENT_TOK_IDENTIFIER) {
	    ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast, true));
    } else {
        // no name, advance
        tokenlist_advance(tokeniser, tindex);
    }

    GET_TOKEN(tokeniser, *tindex, tok);
    if (tok->type == ELEMENT_TOK_COLON) {
        tokenlist_advance(tokeniser, tindex);
        element_ast* type = ast_new_child(ast);
        type->nearest_token = tok;
        ELEMENT_OK_OR_RETURN(parse_typename(tindex, type));
    }
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_portlist(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    assert(tok->type == ELEMENT_TOK_IDENTIFIER || tok->type == ELEMENT_TOK_UNDERSCORE);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_PORTLIST;
    ast->flags = 0;
    do
    {
        element_ast* port = ast_new_child(ast);
        ELEMENT_OK_OR_RETURN(parse_port(tindex, port));
        GET_TOKEN(tokeniser, *tindex, tok);
        port->nearest_token = tok;
    } while (tok->type == ELEMENT_TOK_COMMA && tokenlist_advance(tokeniser, tindex));

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_exprlist(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    assert(token->type == ELEMENT_TOK_BRACKETL);
    TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, token);
    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_EXPRLIST;
    if (token->type != ELEMENT_TOK_BRACKETR) {
        do {
            element_ast* eid = ast_new_child(ast);
            ELEMENT_OK_OR_RETURN(parse_expression(tindex, eid));
            GET_TOKEN(tokeniser, *tindex, token);
            eid->nearest_token = token;
        } while (token->type == ELEMENT_TOK_COMMA && tokenlist_advance(tokeniser, tindex));
    }
    else {
        //todo: not really sure what these errors mean...
        return ELEMENT_ERROR_INVALID_ARCHIVE;
    }
    assert(token->type == ELEMENT_TOK_BRACKETR);
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_call(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    assert(token->type == ELEMENT_TOK_IDENTIFIER || token->type == ELEMENT_TOK_NUMBER);
    // get identifier
    ast_unique_ptr root = ast_new(nullptr, ELEMENT_AST_NODE_CALL);
    root->nearest_token = token;
    if (token->type == ELEMENT_TOK_IDENTIFIER) {
        ELEMENT_OK_OR_RETURN(parse_identifier(tindex, root.get()));
    } else if (token->type == ELEMENT_TOK_NUMBER) {
        // this will change root's type to LITERAL
        ELEMENT_OK_OR_RETURN(parse_literal(tindex, root.get()));
    } else {
        assert(false);
        return ELEMENT_ERROR_INVALID_ARCHIVE; // TODO: error code
    }
    GET_TOKEN(tokeniser, *tindex, token);

    while (token->type == ELEMENT_TOK_DOT || token->type == ELEMENT_TOK_BRACKETL) {
        if (token->type == ELEMENT_TOK_BRACKETL) {
            // call with args
            // TODO: bomb out if we're trying to call a literal
            // add blank "none" if this is a simple call
            if (root->children.empty()) {
                auto blank_ast = ast_new(root.get(), ELEMENT_AST_NODE_NONE);
                blank_ast->nearest_token = token;
                ast_add_child(root.get(), std::move(blank_ast));
            }
            // parse args
            element_ast* cid = ast_new_child(root.get());
            cid->nearest_token = token;
            ELEMENT_OK_OR_RETURN(parse_exprlist(tindex, cid));
        } else if (token->type == ELEMENT_TOK_DOT) {
            // member field access
            ast_unique_ptr callroot = ast_new(nullptr, ELEMENT_AST_NODE_CALL);
            // move existing root into left side of new one
            ast_add_child(callroot.get(), std::move(root));
            tokenlist_advance(tokeniser, tindex);
            ELEMENT_OK_OR_RETURN(parse_identifier(tindex, callroot.get()));
            root = std::move(callroot);
        }
        GET_TOKEN(tokeniser, *tindex, token);
    }
    ast_move(root.get(), ast, false);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_lambda(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    assert(token->type == ELEMENT_TOK_UNDERSCORE);
    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_LAMBDA;

    TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, token);
    if (token->type != ELEMENT_TOK_BRACKETL)
        return ELEMENT_ERROR_INVALID_OPERATION;
    tokenlist_advance(tokeniser, tindex);
    element_ast* ports = ast_new_child(ast);
    ELEMENT_OK_OR_RETURN(parse_portlist(tindex, ports));
    GET_TOKEN(tokeniser, *tindex, token);
    if (token->type != ELEMENT_TOK_BRACKETR)
        return ELEMENT_ERROR_INVALID_OPERATION;
    tokenlist_advance(tokeniser, tindex);
    element_ast* body = ast_new_child(ast);
    body->nearest_token = token;
    ELEMENT_OK_OR_RETURN(parse_body(tindex, body, false));
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_expression(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token); //TODO: JM - CommentInline-fail.ele
    assert(token->type == ELEMENT_TOK_IDENTIFIER || token->type == ELEMENT_TOK_UNDERSCORE || token->type == ELEMENT_TOK_NUMBER);
    ast->nearest_token = token;
    if (token->type == ELEMENT_TOK_IDENTIFIER || token->type == ELEMENT_TOK_NUMBER)
        return parse_call(tindex, ast);
    if (token->type == ELEMENT_TOK_UNDERSCORE)
        return parse_lambda(tindex, ast);
    return ELEMENT_ERROR_INVALID_OPERATION; // TODO: better error code...
}

element_result element_parser_ctx::parse_qualifiers(size_t* tindex, element_ast_flags* flags)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);

    // keep track of previous flags so we can check there are no duplicates
    std::vector<element_ast_flags> qualifier_flags;

    while (tok->type == ELEMENT_TOK_IDENTIFIER) {
        std::string id = tokeniser->text(tok);
        if (id == "intrinsic") {
            bool found_duplicate_intrinsic = false;
            for (element_ast_flags flag : qualifier_flags) {
                if (flag == ELEMENT_AST_FLAG_DECL_INTRINSIC)
                    found_duplicate_intrinsic = true;
            }

            //rather than log an error in here, let's assume the next thing to handle this token will give a useful error message
            if (found_duplicate_intrinsic)
                break;

            *flags |= ELEMENT_AST_FLAG_DECL_INTRINSIC;
            qualifier_flags.push_back(ELEMENT_AST_FLAG_DECL_INTRINSIC);
        } else {
            break;
        }

        TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_declaration(size_t* tindex, element_ast* ast, bool find_return_type)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    assert(tok->type == ELEMENT_TOK_IDENTIFIER);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_DECLARATION;
    ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast));
    GET_TOKEN(tokeniser, *tindex, tok);
    // always create the args node, even if it ends up being none/empty
    element_ast* args = ast_new_child(ast, ELEMENT_AST_NODE_NONE);
    args->nearest_token = tok;
    if (tok->type == ELEMENT_TOK_BRACKETL) {
        // argument list
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_portlist(tindex, args));
        GET_TOKEN(tokeniser, *tindex, tok);
        assert(tok->type == ELEMENT_TOK_BRACKETR);
        TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    }
    else {
        args->flags |= ELEMENT_AST_FLAG_DECL_EMPTY_INPUT;
    }

    auto has_return = tok->type == ELEMENT_TOK_COLON;
    if (has_return) {
        if (find_return_type) {
            // output type
            tokenlist_advance(tokeniser, tindex);
            element_ast* type = ast_new_child(ast);
            type->nearest_token = tok;
            ELEMENT_OK_OR_RETURN(parse_typename(tindex, type));
        }
        else {
            log(ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE, 
                    "A struct cannot have a return type");
            return ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE;
        }
    }
    else
    {
        // implied any output
        element_ast* ret = ast_new_child(ast, ELEMENT_AST_NODE_NONE);
        ret->nearest_token = tok;
        ret->flags = ELEMENT_AST_FLAG_DECL_IMPLICIT_RETURN;	    
    }

	
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_scope(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    if (token->type == ELEMENT_TOK_BRACEL)
        TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, token);

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_SCOPE;
    while (token->type != ELEMENT_TOK_BRACER) {
        element_ast* item = ast_new_child(ast);
        item->nearest_token = token;
        ELEMENT_OK_OR_RETURN(parse_item(tindex, item));
        GET_TOKEN(tokeniser, *tindex, token);
    }
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_body(size_t* tindex, element_ast* ast, bool expr_requires_semi)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    if (token->type == ELEMENT_TOK_BRACEL) {
        // scope (function body)
        ELEMENT_OK_OR_RETURN(parse_scope(tindex, ast));
    } else if (token->type == ELEMENT_TOK_EQUALS) {
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_expression(tindex, ast));
        if (expr_requires_semi) {
            GET_TOKEN(tokeniser, *tindex, token);
            if (token->type == ELEMENT_TOK_SEMICOLON) {
                tokenlist_advance(tokeniser, tindex);
            } else {
                return ELEMENT_ERROR_INVALID_ARCHIVE;
            }
        }
    } else {
        assert(false);
        return ELEMENT_ERROR_INVALID_ARCHIVE;
    }
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_function(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_FUNCTION;
    element_ast* decl = ast_new_child(ast);
    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, decl, true));
    decl->flags = declflags;

    element_ast* bodynode = ast_new_child(ast);
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    bodynode->nearest_token = body;
    if (body->type == ELEMENT_TOK_SEMICOLON) {
        bodynode->type = ELEMENT_AST_NODE_CONSTRAINT;
        if (decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC)) {
            tokenlist_advance(tokeniser, tindex);
        }
        else {
            log(ELEMENT_ERROR_MISSING_FUNCTION_BODY, 
                "Non-intrinsic functions must declare a body");
            return ELEMENT_ERROR_MISSING_FUNCTION_BODY;
        }
    } else {
        // real body of some sort
        ELEMENT_OK_OR_RETURN(parse_body(tindex, bodynode, true));
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_struct(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

	if(token->type == ELEMENT_TOK_EQUALS)
	{
        log(ELEMENT_ERROR_INVALID_IDENTIFIER, 
            "invalid identifier found, cannot use '=' after a struct without an identifier");
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
	}

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_STRUCT;
    element_ast* decl = ast_new_child(ast);
    decl->flags = declflags;

    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, decl, false))

    const auto is_intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    const auto has_portlist = !decl->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);

    //todo: ask craig
    if (!is_intrinsic && !has_portlist)
    {
        log(ELEMENT_ERROR_MISSING_PORTS,
            fmt::format("Port list for struct '{}' is required as it is not intrinsic",
                tokeniser->text(ast->nearest_token)));
        return ELEMENT_ERROR_MISSING_PORTS;
    }

    element_ast* bodynode = ast_new_child(ast);
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    bodynode->nearest_token = body;
    tokenlist_advance(tokeniser, tindex);
    if (body->type == ELEMENT_TOK_SEMICOLON) {
        // constraint
        bodynode->type = ELEMENT_AST_NODE_CONSTRAINT;
    } else if (body->type == ELEMENT_TOK_BRACEL) {
        // scope (struct body)
        ELEMENT_OK_OR_RETURN(parse_scope(tindex, bodynode));
    } else {
        log(ELEMENT_ERROR_UNKNOWN, 
            "unknown error in parse_struct");
        return ELEMENT_ERROR_UNKNOWN;
    }
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_constraint(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    if (token->type == ELEMENT_TOK_EQUALS)
    {
        log(ELEMENT_ERROR_INVALID_IDENTIFIER,
            "invalid identifier found, cannot use '=' after a constraint without an identifier");
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_CONSTRAINT;
    element_ast* decl = ast_new_child(ast);
    decl->flags = declflags;

    // cosntraints can have return types
    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, decl, true))

    const auto is_intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    const auto has_portlist = !decl->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);

    //todo: ask craig, port list for struct 
    if (!is_intrinsic && !has_portlist)
    {
        log(ELEMENT_ERROR_MISSING_PORTS,
            fmt::format("Port list for constraint '{}' is required as it is not intrinsic",
                tokeniser->text(ast->nearest_token)));
        return ELEMENT_ERROR_MISSING_PORTS;
    }

    element_ast* bodynode = ast_new_child(ast);
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    bodynode->nearest_token = body;
    tokenlist_advance(tokeniser, tindex);

    if (body->type == ELEMENT_TOK_SEMICOLON) {
        bodynode->type = ELEMENT_AST_NODE_CONSTRAINT;
    }
    else if (body->type == ELEMENT_TOK_BRACEL) {
        log(ELEMENT_ERROR_CONSTRAINT_HAS_BODY, 
            fmt::format("a body was found for constraint '{}', but constraints cannot have bodies", 
                ast->identifier));
        return ELEMENT_ERROR_CONSTRAINT_HAS_BODY;
    } else {
        log(ELEMENT_ERROR_UNKNOWN, 
            "unknown error parsing constraint");
        return ELEMENT_ERROR_UNKNOWN;
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_namespace(size_t* tindex, element_ast* ast)
{
    tokenlist_advance(tokeniser, tindex);

    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    if (token->type == ELEMENT_TOK_EQUALS)
    {
        log(ELEMENT_ERROR_INVALID_IDENTIFIER,
            "invalid identifier found, cannot use '=' after a namespace without an identifier");
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_NAMESPACE;
    ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast));

    element_ast* scope = ast_new_child(ast);
    ELEMENT_OK_OR_RETURN(parse_scope(tindex, scope));

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_item(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    ast->nearest_token = token;

    //A sole underscore was used as an identifier
    if (token->type == ELEMENT_TOK_UNDERSCORE) {
        log(ELEMENT_ERROR_INVALID_IDENTIFIER,
            fmt::format("Invalid identifier '{}'", tokeniser->text(token)));
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    // either a qualifier, 'struct', 'namespace' or a name; either way...
    assert(token->type == ELEMENT_TOK_IDENTIFIER);

    if (tokeniser->text(token) == "namespace") {
        ELEMENT_OK_OR_RETURN(parse_namespace(tindex, ast));
    } else {
        element_ast_flags flags = 0;
        // parse qualifiers
        ELEMENT_OK_OR_RETURN(parse_qualifiers(tindex, &flags));
        GET_TOKEN(tokeniser, *tindex, token);
        ast->nearest_token = token;
        if (tokeniser->text(token) == "struct") {
            tokenlist_advance(tokeniser, tindex);
            ELEMENT_OK_OR_RETURN(parse_struct(tindex, ast, flags));
        } else if (tokeniser->text(token) == "constraint") {
            tokenlist_advance(tokeniser, tindex);
            ELEMENT_OK_OR_RETURN(parse_constraint(tindex, ast, flags));
        } else {
            ELEMENT_OK_OR_RETURN(parse_function(tindex, ast, flags));
        }
    }
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse(size_t* tindex, element_ast* ast)
{
    size_t tcount;
    const element_token* tok;
    GET_TOKEN_COUNT(tokeniser, tcount);
    GET_TOKEN(tokeniser, *tindex, tok);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_ROOT;
    if (*tindex < tcount && tok->type == ELEMENT_TOK_NONE)
        TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    while (*tindex < tcount) {
        element_ast* item = ast_new_child(ast);
        ELEMENT_OK_OR_RETURN(parse_item(tindex, item));
        if (*tindex < tcount && tok->type == ELEMENT_TOK_NONE)
            TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    }
    return ELEMENT_OK;
}

element_result element_parser_ctx::validate(element_ast* ast)
{
    element_result result = ELEMENT_OK;
    result = validate_type(ast);
	if(result != ELEMENT_OK)
        return result;
	
	const auto length = ast->children.size();
    if (length == 0)
        return result;

	for(auto i = 0; i < length; i++)
	{
		//special case validation
		auto* const child = ast->children[i].get();
		const auto validate_result = validate(child);
        if (result == ELEMENT_OK)
            result = validate_result;
	}
	
    return result;
}

element_result element_parser_ctx::validate_type(element_ast* ast)
{
    element_result result = ELEMENT_OK;
    switch (ast->type)
    {
    case ELEMENT_AST_NODE_PORTLIST:
        return validate_portlist(ast);

    case ELEMENT_AST_NODE_STRUCT:
        return validate_struct(ast);
    	
    case ELEMENT_AST_NODE_ROOT:
    case ELEMENT_AST_NODE_SCOPE:
        return validate_scope(ast);
    default: break;
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::validate_portlist(element_ast* ast)
{
    const auto length = ast->children.size();
    if (length == 0) {

        log(ELEMENT_ERROR_MISSING_PORTS, fmt::format("Port list cannot be empty"));
        return ELEMENT_ERROR_MISSING_PORTS;
    }

    // ensure port identifiers are all unique
    element_result result = ELEMENT_OK;
    for (unsigned int i = 0; i < ast->children.size(); ++i)
    {
        for (unsigned int j = i; j < ast->children.size(); ++j)
        {
            if (i != j &&  // not the same port
                ast->children[i]->identifier == ast->children[j]->identifier)
            {
                log(ELEMENT_ERROR_MULTIPLE_DEFINITIONS,
                    fmt::format("Parameter {} and {} in the port list of function '{}' have the same identifier '{}'",
                        i, j, ast->parent->identifier, ast->children[i]->identifier));
                result = ELEMENT_ERROR_MULTIPLE_DEFINITIONS;
            }
        }
    }

    return result;
}

element_result element_parser_ctx::validate_struct(element_ast* ast)
{
    element_result result = ELEMENT_OK;
    assert(ast->children.size() > ast_idx::fn::declaration);
    if (ast->children.size() > ast_idx::fn::body) 
    {
        auto* decl = ast->children[ast_idx::fn::declaration].get();
        assert(decl->type == ELEMENT_AST_NODE_DECLARATION);
    	
        auto identifier = decl->identifier;
        auto* body = ast->children[ast_idx::fn::body].get();
        assert(body->type == ELEMENT_AST_NODE_SCOPE);
    	
        for (auto& child : body->children) {
        	
            assert(child->children.size() > ast_idx::fn::declaration);
            auto* child_decl = child->children[ast_idx::fn::declaration].get();
            assert(child_decl->type == ELEMENT_AST_NODE_DECLARATION);
        	
            if (identifier == child_decl->identifier)
            {
                log(ELEMENT_ERROR_INVALID_IDENTIFIER,
                    fmt::format("Struct identifier '{}' detected in scope '{}'",
                        ast->identifier, ast->identifier));
                result = ELEMENT_ERROR_INVALID_IDENTIFIER;
            }
        }
    }

    return result;
}

element_result element_parser_ctx::validate_scope(element_ast* ast)
{
    element_result result = ELEMENT_OK;

    std::vector<std::string> names;
    for (auto& child : ast->children) {

        auto* child_decl = child->children[ast_idx::fn::declaration].get();
        auto child_identifier = child_decl->identifier;
        auto it = std::find(names.begin(), names.end(), child_identifier);
    	if(it != names.end())
    	{
            log(ELEMENT_ERROR_MULTIPLE_DEFINITIONS,
                fmt::format("Duplicate declaration '{}' detected in scope", child_identifier));
            result = ELEMENT_ERROR_MULTIPLE_DEFINITIONS;
    	}
        else
        {
            names.push_back(child_decl->identifier);
        }
    }
	
    return ELEMENT_OK;
}

element_result element_parser_ctx::ast_build()
{
    // don't use ast_new here, as we need to return this pointer to the user
    root = new element_ast(nullptr);
    size_t index = 0;
    auto result = parse(&index, root);
    if (result != ELEMENT_OK) {
        element_ast_delete(root);
        root = nullptr;
        return result;
    }

    result = validate(root);
    return result;
}

void element_ast_delete(element_ast* ast)
{
    if (ast)
        ast_clear(ast);
    delete ast;
}

#define PRINTCASE(a) case a: c = #a; break;
static element_result ast_print_depth(element_ast* ast, int depth)
{
    for (int i = 0; i < depth; ++i) printf("  ");
    if (ast->type == ELEMENT_AST_NODE_LITERAL) {
        printf("LITERAL: %f", ast->literal);
    } else if (ast->type == ELEMENT_AST_NODE_IDENTIFIER) {
        printf("IDENTIFIER: %s", ast->identifier.c_str());
    } else if (ast->type == ELEMENT_AST_NODE_DECLARATION) {
        auto intrinsic = ast->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
        intrinsic
            ? printf("INTRINSIC DECLARATION: %s", ast->identifier.c_str())
            : printf("DECLARATION: %s", ast->identifier.c_str());
    } else if (ast->type == ELEMENT_AST_NODE_NAMESPACE) {
        printf("NAMESPACE: %s", ast->identifier.c_str());
    } else if (ast->type == ELEMENT_AST_NODE_CALL) {
        printf("CALL: %s", ast->identifier.c_str());
    } else if (ast->type == ELEMENT_AST_NODE_PORT) {
        printf("PORT: %s", ast->identifier.c_str());
    }
    else if (ast->type == ELEMENT_AST_NODE_NONE) {
        if (ast->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT))
            printf("EMPTY INPUT");
        else if (ast->has_flag(ELEMENT_AST_FLAG_DECL_IMPLICIT_RETURN))
            printf("IMPLICIT RETURN");
        else
            printf("NONE");
    }
    else {
        char* c;
        switch (ast->type) {
            PRINTCASE(ELEMENT_AST_NODE_ROOT);
            PRINTCASE(ELEMENT_AST_NODE_SCOPE);
            PRINTCASE(ELEMENT_AST_NODE_CONSTRAINT);
            PRINTCASE(ELEMENT_AST_NODE_FUNCTION);
            PRINTCASE(ELEMENT_AST_NODE_STRUCT);
            PRINTCASE(ELEMENT_AST_NODE_EXPRESSION);
            PRINTCASE(ELEMENT_AST_NODE_EXPRLIST);
            PRINTCASE(ELEMENT_AST_NODE_PORTLIST);
            PRINTCASE(ELEMENT_AST_NODE_TYPENAME);
            PRINTCASE(ELEMENT_AST_NODE_LAMBDA);
            default: c = "ELEMENT_AST_NODE_<UNKNOWN>"; break;
        }
        printf("%s", c + strlen("ELEMENT_AST_NODE_"));
    }
    printf("\n");

    for (const auto& child : ast->children)
        ast_print_depth(child.get(), depth + 1);
    return ELEMENT_OK;
}

element_result element_ast_print(element_ast* ast)
{
    return ast_print_depth(ast, 0);
}

element_ast::walk_step element_ast::walk(const element_ast::walker& fn)
{
    walk_step s = fn(this);
    switch (s) {
    case walk_step::step_in:
    {
        auto it = children.begin();
        while (it != children.end() && fn(it->get()) == walk_step::next)
            ++it;
        return walk_step::next;
    }
    case walk_step::next:
    case walk_step::step_out:
    case walk_step::stop:
    default:
        return s;
    }
}

element_ast::walk_step element_ast::walk(const element_ast::const_walker& fn) const
{
    walk_step s = fn(this);
    switch (s) {
    case walk_step::step_in:
    {
        auto it = children.begin();
        while (it != children.end() && static_cast<const element_ast*>(it->get())->walk(fn) == walk_step::next)
            ++it;
        return walk_step::next;
    }
    case walk_step::next:
    case walk_step::step_out:
    case walk_step::stop:
    default:
        return s;
    }
}

void element_parser_ctx::log(int message_code, const std::string& message) const
{
    if (!log_callback)
        return;

    auto log = element_log_message();
    log.message = message.c_str();
    log.message_code = message_code;
    log.line = -1;
    log.column = -1;
    log.length = -1;
    log.stage = ELEMENT_STAGE_PARSER;
    log.filename = nullptr;
    log.related_log_message = nullptr;

    log_callback(&log);
}

void element_parser_ctx::log(const element_log_message& message) const
{
    if (!log_callback)
        return;

    log_callback(&message);
}
