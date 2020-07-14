#include "element/ast.h"

//STD
#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include <unordered_set>

//LIBS
#include <fmt/format.h>
#include "MemoryPool.h"

//SELF
#include "element/token.h"
#include "ast/ast_internal.hpp"
#include "ast/ast_indexes.hpp"
#include "token_internal.hpp"
#include "configuration.hpp"
#include "log_errors.hpp"

//static const std::string intrinsic_qualifier = "intrinsic";
//static const std::string namespace_qualifier = "namespace";
//static const std::string struct_qualifier = "struct";
//static const std::string constraint_qualifier = "constraint";
//static const std::string return_keyword = "return";

static std::unordered_set<std::string> qualifiers {"intrinsic"};
static std::unordered_set<std::string> constructs{ "struct", "namespace", "constraint"};
static std::unordered_set<std::string> reserved_args{};
static std::unordered_set<std::string> reserved_names {"return"};

static element_result check_reserved_words(const std::string& text, bool allow_reserved_arg, bool allow_reserved_names)
{
    const bool is_not_a_reserved_qualifier = qualifiers.count(text) == 0;
    const bool is_not_a_reserved_construct = constructs.count(text) == 0;
    const bool is_not_a_reserved_arg = allow_reserved_arg || reserved_args.count(text) == 0;
    const bool is_not_a_reserved_name = allow_reserved_names || reserved_names.count(text) == 0;
    const bool valid = is_not_a_reserved_qualifier
                    && is_not_a_reserved_construct
                    && is_not_a_reserved_arg
                    && is_not_a_reserved_name;

    return valid ? ELEMENT_OK : ELEMENT_ERROR_RESERVED_IDENTIFIER;
}

//
// Token helpers
//
#define GET_TOKEN(tctx, tindex, tok) ELEMENT_OK_OR_RETURN(element_tokeniser_get_token((tctx), (tindex), &(tok), nullptr))
#define GET_TOKEN_CUSTOM_MSG(tctx, tindex, tok, msg) ELEMENT_OK_OR_RETURN(element_tokeniser_get_token((tctx), (tindex), &(tok), msg))
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
    while (*tindex < tcount - 1 && tok->type == ELEMENT_TOK_NONE) {
        ++(*tindex);
        GET_TOKEN(tctx, *tindex, tok);
    }
	
    return (*tindex < tcount);
}

#define TOKENLIST_ADVANCE_AND_UPDATE(tctx, tindex, tok) \
{ tokenlist_advance((tctx), (tindex)); ELEMENT_OK_OR_RETURN(element_tokeniser_get_token((tctx), *(tindex), &(tok), nullptr)); }

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
    node->flags = 0;
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
    for (auto& child : to->children)
        child->parent = to;

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
    //the caller should ensure it's a number
    assert(token->type == ELEMENT_TOK_NUMBER);
    ast->type = ELEMENT_AST_NODE_LITERAL;
    ast->literal = std::stof(tokeniser->text(token));
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_identifier(size_t* tindex, element_ast* ast, bool allow_reserved_args, bool allow_reserved_names)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    ast->nearest_token = token;
    ast->identifier.assign(tokeniser->text(token));

    if (token->type != ELEMENT_TOK_IDENTIFIER)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_identifier_failed,
            ast->identifier);
    }

    tokenlist_advance(tokeniser, tindex);

    const auto result = check_reserved_words(ast->identifier, allow_reserved_args, allow_reserved_names);
    if (result != ELEMENT_OK) {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_identifier_reserved,
            ast->identifier);
    }

    return result;
}

element_result element_parser_ctx::parse_typename(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_TYPENAME;

    if (tok->type != ELEMENT_TOK_IDENTIFIER) {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_typename_not_identifier,
            tokeniser->text(tok));
    }

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
        if (tok->type == ELEMENT_TOK_DOT) {
            TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
        }
    }

    return result;
}

element_result element_parser_ctx::parse_port(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_PORT;
    if (tok->type == ELEMENT_TOK_IDENTIFIER) {
	    ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast, true));
    } else if (tok->type == ELEMENT_TOK_UNDERSCORE) {
        // no name, advance
        tokenlist_advance(tokeniser, tindex);
    } else {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_port_failed,
            tokeniser->text(tok));
    }

    GET_TOKEN(tokeniser, *tindex, tok);
    if (tok->type == ELEMENT_TOK_COLON) {
        tokenlist_advance(tokeniser, tindex);
        element_ast* type = ast_new_child(ast, ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        type->nearest_token = tok;
        ELEMENT_OK_OR_RETURN(parse_typename(tindex, type));
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_portlist(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    //the previous token is the bracket
    element_tokeniser_get_token(tokeniser, *tindex-1, &ast->nearest_token, nullptr); 
    ast->type = ELEMENT_AST_NODE_PORTLIST;
    ast->flags = 0;
    do
    {
        element_ast* port = ast_new_child(ast);
        ELEMENT_OK_OR_RETURN(parse_port(tindex, port));
        GET_TOKEN(tokeniser, *tindex, tok);
    } while (tok->type == ELEMENT_TOK_COMMA && tokenlist_advance(tokeniser, tindex));

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_exprlist(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    //the caller should ensure it's a '('
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
        //should be '(' for previous and ')' for current
        const element_token* previous_token;
        GET_TOKEN(tokeniser, *tindex - 1, previous_token);
        auto info = build_source_info(src_context.get(), previous_token, token->tok_len);

        return log_error(
            logger.get(),
            info,
            element::log_error_message_code::parse_exprlist_empty,
            ast->parent->identifier);
    }

    if (token->type != ELEMENT_TOK_BRACKETR)
    {
        element_log_message msg;
        const std::string message = fmt::format("expected to find a ')' at the end of the call to '{}', but found '{}' instead",
            ast->parent->identifier, tokeniser->text(token));
        const std::string line_in_source = tokeniser->text_on_line(token->line);
        msg.line_in_source = line_in_source.c_str();
        msg.message = message.c_str();
        msg.filename = token->file_name;
        msg.length = token->tok_len;
        msg.line = token->line;
        msg.message_code = ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL;
        msg.related_log_message = nullptr;
        msg.stage = ELEMENT_STAGE_PARSER;
        msg.character = token->character;
        tokeniser->logger->log(msg);
        return ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL;
    }
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_call(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    /* The first AST node is either LITERAL or CALL
     * LITERAL or CALL can have children, which indicates that they are the start of a chain (literals are always at the start of a chain)
     *  e.g. 180.add(2).add(3) is a LITERAL 180 with four children, CALL ADD, EXPRLIST, CALL ADD, EXPRLIST
     * an EXPRLIST indicates parenthesis (). In the above situation, each EXPRLIST will have a child which is another parse_call()
     * The first has a child LITERAL 2, the other LITERAL 3
     * In situations where there are multiple arguments, e.g. Num.add(Num.add(1, 2), Num.mul(Num.pi, Num.pi)).mul(1.add(2)), this translates to the following:
     * CALL: Num
     *     CALL: add
     *     EXPRLIST
     *         CALL: Num
     *             CALL: add
     *             EXPRLIST
     *                 LITERAL 1
     *                 LITERAL 2
     *         CALL: Num
     *             CALL: mul
     *             EXPRLIST
     *                 CALL: Num
     *                     CALL: pi
     *                 CALL: Num
     *                     CALL: pi
     *     CALL: mul
     *     EXPRLIST
     *         LITERAL 1
     *             CALL: add
     *             EXPRLIST
     *                 LITERAL 2
     */

    element_ast* root = ast;

    if (token->type == ELEMENT_TOK_IDENTIFIER) {
        // get identifier
        ELEMENT_OK_OR_RETURN(parse_identifier(tindex, root));
        root->type = ELEMENT_AST_NODE_CALL;
    } else if (token->type == ELEMENT_TOK_NUMBER) {
        // this will change root's type to LITERAL
        ELEMENT_OK_OR_RETURN(parse_literal(tindex, root));
    } else {
        element_log_message msg;
        const std::string message =
            fmt::format("expected to find an identifier or number in the contents of the call to '{}', but found '{}' instead.",
                ast->parent->identifier, tokeniser->text(token));
        const std::string line_in_source = tokeniser->text_on_line(token->line);
        msg.line_in_source = line_in_source.c_str();
        msg.message = message.c_str();
        msg.filename = token->file_name;
        msg.character = token->character;
        msg.length = token->tok_len;
        msg.line = token->line;
        msg.message_code = ELEMENT_ERROR_INVALID_CONTENTS_FOR_CALL;
        msg.related_log_message = nullptr;
        msg.stage = ELEMENT_STAGE_PARSER;
        tokeniser->logger->log(msg);
        return ELEMENT_ERROR_INVALID_CONTENTS_FOR_CALL;
    }

    GET_TOKEN(tokeniser, *tindex, token);

    while (token->type == ELEMENT_TOK_DOT || token->type == ELEMENT_TOK_BRACKETL) {
        if (token->type == ELEMENT_TOK_BRACKETL) {
            // call with args
            // TODO: bomb out if we're trying to call a literal, keep track of previous node
            
            const auto call_node = ast_new_child(root);
            ELEMENT_OK_OR_RETURN(parse_exprlist(tindex, call_node));
        	
        } else if (token->type == ELEMENT_TOK_DOT){
            //advance over the dot so we're now at (what should be) the identifier token
            tokenlist_advance(tokeniser, tindex);

            auto indexing_node = ast_new_child(root, ELEMENT_AST_NODE_CALL);
            ELEMENT_OK_OR_RETURN(parse_identifier(tindex, indexing_node));
        }

        GET_TOKEN(tokeniser, *tindex, token);
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_lambda(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    //the caller should ensure it's an underscore
    assert(token->type == ELEMENT_TOK_UNDERSCORE);
    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_LAMBDA;

    TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, token);
    //todo: logging
    if (token->type != ELEMENT_TOK_BRACKETL)
        return ELEMENT_ERROR_INVALID_OPERATION;
    tokenlist_advance(tokeniser, tindex);
    element_ast* ports = ast_new_child(ast);
    ELEMENT_OK_OR_RETURN(parse_portlist(tindex, ports));
    GET_TOKEN(tokeniser, *tindex, token);
    //todo: logging
    if (token->type != ELEMENT_TOK_BRACKETR)
        return ELEMENT_ERROR_INVALID_OPERATION;
    tokenlist_advance(tokeniser, tindex);

    GET_TOKEN(tokeniser, *tindex, token);
    auto has_return = token->type == ELEMENT_TOK_COLON;
    if (has_return) 
    {
        tokenlist_advance(tokeniser, tindex);
        element_ast* type = ast_new_child(ast);
        type->nearest_token = token;
        ELEMENT_OK_OR_RETURN(parse_typename(tindex, type));
    }

    element_ast* body = ast_new_child(ast);
    body->nearest_token = token;
    ELEMENT_OK_OR_RETURN(parse_body(tindex, body, false));
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_expression(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token); //TODO: JM - CommentInline-fail.ele
    ast->nearest_token = token;

    if (token->type == ELEMENT_TOK_IDENTIFIER || token->type == ELEMENT_TOK_NUMBER)
        return parse_call(tindex, ast);

    if (token->type == ELEMENT_TOK_UNDERSCORE)
        return parse_lambda(tindex, ast);

    log(ELEMENT_ERROR_INVALID_EXPRESSION, fmt::format("failed to parse the expression '{}'.\nnote: it must start with an identifier, an underscore, or a number.",
        tokeniser->text(token)), ast);
    return ELEMENT_ERROR_INVALID_EXPRESSION;
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
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_DECLARATION;

    if (tok->type != ELEMENT_TOK_IDENTIFIER)
    {
        log(ELEMENT_ERROR_INVALID_IDENTIFIER, "found '{}' when parsing a declaration instead of a valid identifier", ast);
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    bool function_declaration = false;
    if (ast->parent->type == ELEMENT_AST_NODE_FUNCTION)
        function_declaration = true;

    //If a function declaration identifier in another function or lambdas scope is "return" then that's valid, otherwise not
    const auto allow_reserved_names = function_declaration && (ast_node_in_function_scope(ast->parent) || ast_node_in_lambda_scope(ast->parent));
    ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast, false, allow_reserved_names));

    GET_TOKEN(tokeniser, *tindex, tok);
    // always create the args node, even if it ends up being none/empty
    element_ast* args = ast_new_child(ast);
    args->nearest_token = tok;
    if (tok->type == ELEMENT_TOK_BRACKETL) {
        // argument list
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_portlist(tindex, args));

        GET_TOKEN(tokeniser, *tindex, tok);
        if (tok->type != ELEMENT_TOK_BRACKETR) {
            //todo: log function for tokens
            element_log_message msg;
            const std::string message = fmt::format("found '{}' at the end of the declaration for '{}' with a portlist. expected ')'",
                tokeniser->text(tok), ast->identifier);
            const std::string line_in_source = tokeniser->text_on_line(tok->line);
            msg.line_in_source = line_in_source.c_str();
            msg.message = message.c_str();
            msg.filename = tok->file_name;
            msg.length = tok->tok_len;
            msg.line = tok->line;
            msg.message_code = ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST;
            msg.related_log_message = nullptr;
            msg.stage = ELEMENT_STAGE_PARSER;
            msg.character = tok->character;
            tokeniser->logger->log(msg);
            return ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST;
        }

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
                    "A struct cannot have a return type",
                ast);
            return ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE;
        }
    }
    else
    {
        // implied any output
        element_ast* ret = ast_new_child(ast, ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
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

element_result element_parser_ctx::parse_body(size_t* tindex, element_ast* ast, bool expr_requires_semicolon)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    if (token->type == ELEMENT_TOK_BRACEL) {
        // scope (function body)
        ELEMENT_OK_OR_RETURN(parse_scope(tindex, ast));
    } else if (token->type == ELEMENT_TOK_EQUALS) {
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_expression(tindex, ast));
        if (expr_requires_semicolon) {
            GET_TOKEN(tokeniser, *tindex, token);
            if (token->type == ELEMENT_TOK_SEMICOLON) {
                tokenlist_advance(tokeniser, tindex);
            } else {
                //todo: log function for token
                element_log_message msg;
                const std::string message = fmt::format("expected to find a ';' at the end of the expression for '{}', but found '{}' instead.",
                    ast->parent->children[ast_idx::function::declaration]->identifier, tokeniser->text(token));
                const std::string line_in_source = tokeniser->text_on_line(token->line);
                msg.line_in_source = line_in_source.c_str();
                msg.message = message.c_str();
                msg.filename = token->file_name;
                msg.length = token->tok_len;
                msg.line = token->line;
                msg.message_code = ELEMENT_ERROR_MISSING_SEMICOLON;
                msg.related_log_message = nullptr;
                msg.stage = ELEMENT_STAGE_PARSER;
                msg.character = token->character;
                tokeniser->logger->log(msg);
                return ELEMENT_ERROR_MISSING_SEMICOLON;
            }
        }
    } else {
        if (ast->parent->type == ELEMENT_AST_NODE_FUNCTION) {
            log(ELEMENT_ERROR_MISSING_FUNCTION_BODY, "expecting function body but none was found.", ast);
            return ELEMENT_ERROR_MISSING_FUNCTION_BODY;
        }
        log(ELEMENT_ERROR_MISSING_BODY, "expected to find a body but none was found.", ast);
        return ELEMENT_ERROR_MISSING_BODY;
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_function(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_FUNCTION;
    element_ast* const declaration = ast_new_child(ast);
    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, declaration, true));
    declaration->flags = declflags;

    auto* body_node = ast_new_child(ast);
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    body_node->nearest_token = body;
    if (body->type == ELEMENT_TOK_SEMICOLON) {
        body_node->type = ELEMENT_AST_NODE_NO_BODY;
        if (declaration->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC)) {
            tokenlist_advance(tokeniser, tindex);
        }
        else {
            log(ELEMENT_ERROR_MISSING_FUNCTION_BODY, 
                "non-intrinsic functions must declare a body",
                ast);
            return ELEMENT_ERROR_MISSING_FUNCTION_BODY;
        }
    } else {
        // real body of some sort
        ELEMENT_OK_OR_RETURN(parse_body(tindex, body_node, true));
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
            "invalid identifier found, cannot use '=' after a struct without an identifier",
            ast);
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
	}

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_STRUCT;
    element_ast* declaration = ast_new_child(ast);
    declaration->flags = declflags;

    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, declaration, false))

    const auto is_intrinsic = declaration->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    const auto has_portlist = !declaration->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);

    //todo: ask craig
    if (!is_intrinsic && !has_portlist)
    {
        log(ELEMENT_ERROR_MISSING_PORTS,
            fmt::format("portlist for struct '{}' is required as it is not intrinsic.",
                tokeniser->text(ast->nearest_token)),
            ast);
        return ELEMENT_ERROR_MISSING_PORTS;
    }

    element_ast* body_node = ast_new_child(ast);
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    body_node->nearest_token = body;
    tokenlist_advance(tokeniser, tindex);
    if (body->type == ELEMENT_TOK_SEMICOLON) {
        // constraint
        body_node->type = ELEMENT_AST_NODE_NO_BODY;
    } else if (body->type == ELEMENT_TOK_BRACEL) {
        // scope (struct body)
        ELEMENT_OK_OR_RETURN(parse_scope(tindex, body_node));
    } else {
        log(ELEMENT_ERROR_UNKNOWN, 
            "unknown error when parsing the body of a struct",
            body_node);
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
            "invalid identifier found, cannot use '=' after a constraint without an identifier",
            ast);
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_CONSTRAINT;
    auto* declaration = ast_new_child(ast);
    declaration->flags = declflags;

    // constraints can have return types
    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, declaration, true))

    const auto is_intrinsic = declaration->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    const auto has_portlist = !declaration->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);

    //todo: ask craig, port list for struct 
    if (!is_intrinsic && !has_portlist)
    {
        log(ELEMENT_ERROR_MISSING_PORTS,
            fmt::format("Port list for constraint '{}' is required as it is not intrinsic",
                tokeniser->text(ast->nearest_token)),
            ast);
        return ELEMENT_ERROR_MISSING_PORTS;
    }

    element_ast* body_node = ast_new_child(ast);
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    body_node->nearest_token = body;
    tokenlist_advance(tokeniser, tindex);

    if (body->type == ELEMENT_TOK_SEMICOLON) {
        body_node->type = ELEMENT_AST_NODE_NO_BODY;
    }
    else if (body->type == ELEMENT_TOK_BRACEL) {
        log(ELEMENT_ERROR_CONSTRAINT_HAS_BODY, 
            fmt::format("a body was found for constraint '{}', but constraints cannot have bodies", 
                ast->identifier),
            ast);
        return ELEMENT_ERROR_CONSTRAINT_HAS_BODY;
    } else {
        log(ELEMENT_ERROR_UNKNOWN, 
            "unknown error parsing constraint",
            ast);
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
            "invalid identifier found, cannot use '=' after a namespace without an identifier",
            ast);
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

    // either a qualifier, 'struct', 'namespace' or a name; either way...
	if(token->type != ELEMENT_TOK_IDENTIFIER)
	{
        log(ELEMENT_ERROR_INVALID_IDENTIFIER,
            fmt::format("expected identifier, but found '{}' instead.", tokeniser->text(token)),
            ast);
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
	}

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

#pragma region validation

//TODO: Consider shifting validation from ast to obj_model
element_result element_parser_ctx::validate(element_ast* ast)
{
    element_result result = ELEMENT_OK;
    result = validate_type(ast);
    if (result != ELEMENT_OK)
        return result;

    const auto length = ast->children.size();
    if (length == 0)
        return result;

    for (auto i = 0; i < length; i++)
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
        log(ELEMENT_ERROR_MISSING_PORTS, "portlist cannot be empty", ast);
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
                    fmt::format("parameter '{}' and '{}' in the portlist of function '{}' have the same identifier '{}'",
                        i, j, ast->parent->identifier, ast->children[i]->identifier),
                    ast->children[i].get());
                result = ELEMENT_ERROR_MULTIPLE_DEFINITIONS;
            }
        }
    }

    return result;
}

element_result element_parser_ctx::validate_struct(element_ast* ast)
{
    element_result result = ELEMENT_OK;
    assert(ast->children.size() > ast_idx::function::declaration);
    if (ast->children.size() > ast_idx::function::body)
    {
        auto* declaration = ast->children[ast_idx::function::declaration].get();
        assert(declaration->type == ELEMENT_AST_NODE_DECLARATION);

        auto identifier = declaration->identifier;
        auto* body = ast->children[ast_idx::function::body].get();

        if (body->type == ELEMENT_AST_NODE_SCOPE)
        {
            for (auto& child : body->children)
            {
                assert(child->children.size() > ast_idx::function::declaration);
                auto* child_declaration = child->children[ast_idx::function::declaration].get();
                assert(child_declaration->type == ELEMENT_AST_NODE_DECLARATION || child_declaration->type == ELEMENT_AST_NODE_SCOPE);

                if (identifier == child_declaration->identifier)
                {
                    log(ELEMENT_ERROR_INVALID_IDENTIFIER,
                        fmt::format("struct identifier '{}' detected in scope '{}'",
                            ast->identifier, ast->identifier),
                        ast);
                    result = ELEMENT_ERROR_INVALID_IDENTIFIER;
                }
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

        if (child->type != ELEMENT_AST_NODE_FUNCTION)
            continue; //TODO: Handle other types

        auto* child_declaration = child->children[ast_idx::function::declaration].get();
        auto child_identifier = child_declaration->identifier;
        auto it = std::find(names.begin(), names.end(), child_identifier);
        if (it != names.end())
        {
            log(ELEMENT_ERROR_MULTIPLE_DEFINITIONS,
                fmt::format("duplicate declaration '{}' detected in scope", child_identifier),
                child_declaration);
            result = ELEMENT_ERROR_MULTIPLE_DEFINITIONS;
        }
        else
        {
            names.push_back(child_declaration->identifier);
        }
    }

    return result;
}

#pragma endregion validation

//TODO: Potential zombie code
//element_ast::walk_step element_ast::walk(const element_ast::walker& fn)
//{
//    walk_step s = fn(this);
//    switch (s) {
//    case walk_step::step_in:
//    {
//        auto it = children.begin();
//        while (it != children.end() && fn(it->get()) == walk_step::next)
//            ++it;
//        return walk_step::next;
//    }
//    case walk_step::next:
//    case walk_step::step_out:
//    case walk_step::stop:
//    default:
//        return s;
//    }
//}
//
//element_ast::walk_step element_ast::walk(const element_ast::const_walker& fn) const
//{
//    walk_step s = fn(this);
//    switch (s) {
//    case walk_step::step_in:
//    {
//        auto it = children.begin();
//        while (it != children.end() && static_cast<const element_ast*>(it->get())->walk(fn) == walk_step::next)
//            ++it;
//        return walk_step::next;
//    }
//    case walk_step::next:
//    case walk_step::step_out:
//    case walk_step::stop:
//    default:
//        return s;
//    }
//}

void element_parser_ctx::log(int message_code, const std::string& message, const element_ast* nearest_ast) const
{
    if (logger == nullptr)
        return;

    logger->log(*this, message_code, message, nearest_ast);
}

void element_parser_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return;

    logger->log(message, message_stage::ELEMENT_STAGE_MISC);
}
