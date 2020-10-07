#include "ast/parser_internal.hpp"

//STD
#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include <unordered_set>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/token.h"
#include "ast/ast_internal.hpp"
#include "ast/ast_indexes.hpp"
#include "token_internal.hpp"
#include "log_errors.hpp"

static std::unordered_set<std::string> qualifiers{ "intrinsic" };
static std::unordered_set<std::string> constructs{ "struct", "namespace", "constraint" };
static std::unordered_set<std::string> reserved_args{};
static std::unordered_set<std::string> reserved_names{ "return" };

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
    while (*tindex < tcount - 1 && tok->type == ELEMENT_TOK_NONE)
    {
        ++(*tindex);
        GET_TOKEN(tctx, *tindex, tok);
    }

    return (*tindex < tcount);
}

#define TOKENLIST_ADVANCE_AND_UPDATE(tctx, tindex, tok)                                        \
    {                                                                                          \
        tokenlist_advance((tctx), (tindex));                                                   \
        ELEMENT_OK_OR_RETURN(element_tokeniser_get_token((tctx), *(tindex), &(tok), nullptr)); \
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
    if (result != ELEMENT_OK)
    {
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

    auto* expr = ast->new_child();
    element_result result = parse_expression(tindex, expr);
    if (result != ELEMENT_OK)
    {
        //todo: change error from identifier to invalid expression
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_typename_not_identifier,
            tokeniser->text(tok));
    }

    return result;
}

element_result element_parser_ctx::parse_port(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    ast->nearest_token = tok;
    ast->type = ELEMENT_AST_NODE_PORT;
    if (tok->type == ELEMENT_TOK_IDENTIFIER)
    {
        ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast, true))
    }
    else if (tok->type == ELEMENT_TOK_UNDERSCORE)
    {
        // no name, advance
        tokenlist_advance(tokeniser, tindex);
    }
    else
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_port_failed,
            tokeniser->text(tok));
    }

    auto* const type = ast->new_child(ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
    GET_TOKEN(tokeniser, *tindex, tok)
    type->nearest_token = tok;
    if (tok->type == ELEMENT_TOK_COLON)
    {
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_typename(tindex, type))
    }

    auto* const default_value = ast->new_child(ELEMENT_AST_NODE_UNSPECIFIED_DEFAULT);
    GET_TOKEN(tokeniser, *tindex, tok)
    default_value->nearest_token = tok;
    if (tok->type == ELEMENT_TOK_EQUALS)
    {
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_expression(tindex, default_value))
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_portlist(size_t* tindex, element_ast* ast)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);
    //the previous token is the bracket
    element_tokeniser_get_token(tokeniser, *tindex - 1, &ast->nearest_token, nullptr);
    ast->type = ELEMENT_AST_NODE_PORTLIST;
    ast->flags = 0;
    do
    {
        element_ast* port = ast->new_child();
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
    if (token->type != ELEMENT_TOK_BRACKETR)
    {
        do
        {
            element_ast* eid = ast->new_child();
            ELEMENT_OK_OR_RETURN(parse_expression(tindex, eid));
            GET_TOKEN(tokeniser, *tindex, token);
            eid->nearest_token = token;
        } while (token->type == ELEMENT_TOK_COMMA && tokenlist_advance(tokeniser, tindex));
    }
    else
    {
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
        return log_error(
            logger.get(),
            src_context.get(),
            token,
            element::log_error_message_code::parse_exprlist_missing_closing_parenthesis,
            ast->parent->identifier,
            tokeniser->text(token));
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

    if (token->type == ELEMENT_TOK_IDENTIFIER)
    {
        // get identifier
        ELEMENT_OK_OR_RETURN(parse_identifier(tindex, root));
        root->type = ELEMENT_AST_NODE_CALL;
    }
    else if (token->type == ELEMENT_TOK_NUMBER)
    {
        // this will change root's type to LITERAL
        ELEMENT_OK_OR_RETURN(parse_literal(tindex, root));
    }
    else
    {
        return log_error(
            logger.get(),
            src_context.get(),
            token,
            element::log_error_message_code::parse_call_invalid_expression,
            ast->parent->identifier,
            tokeniser->text(token));
    }

    GET_TOKEN(tokeniser, *tindex, token);

    while (token->type == ELEMENT_TOK_DOT || token->type == ELEMENT_TOK_BRACKETL)
    {
        if (token->type == ELEMENT_TOK_BRACKETL)
        {
            // call with args
            // TODO: bomb out if we're trying to call a literal, keep track of previous node

            const auto call_node = root->new_child();
            ELEMENT_OK_OR_RETURN(parse_exprlist(tindex, call_node));
        }
        else if (token->type == ELEMENT_TOK_DOT)
        {
            //advance over the dot so we're now at (what should be) the identifier token
            tokenlist_advance(tokeniser, tindex);

            auto indexing_node = root->new_child(ELEMENT_AST_NODE_CALL);
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
    element_ast* ports = ast->new_child();
    ELEMENT_OK_OR_RETURN(parse_portlist(tindex, ports));
    GET_TOKEN(tokeniser, *tindex, token);
    //todo: logging
    if (token->type != ELEMENT_TOK_BRACKETR)
        return ELEMENT_ERROR_INVALID_OPERATION;
    tokenlist_advance(tokeniser, tindex);

    GET_TOKEN(tokeniser, *tindex, token);
    element_ast* type = ast->new_child();
    type->nearest_token = token;
    const auto has_return = token->type == ELEMENT_TOK_COLON;
    if (has_return)
    {
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_typename(tindex, type));
    }
    else
    {
        type->type = ELEMENT_AST_NODE_UNSPECIFIED_TYPE;
    }

    element_ast* body = ast->new_child();
    body->nearest_token = token;
    ELEMENT_OK_OR_RETURN(parse_body(tindex, body));
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_expression(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    ast->nearest_token = token;

    if (token->type == ELEMENT_TOK_IDENTIFIER || token->type == ELEMENT_TOK_NUMBER)
        return parse_call(tindex, ast);

    if (token->type == ELEMENT_TOK_UNDERSCORE)
        return parse_lambda(tindex, ast);

    if (token->type == ELEMENT_TOK_BRACEL)
        return parse_anonymous_block(tindex, ast);

    return log_error(
        logger.get(),
        src_context.get(),
        ast,
        element::log_error_message_code::parse_expression_failed,
        tokeniser->text(token));
}

element_result element_parser_ctx::parse_qualifiers(size_t* tindex, element_ast_flags* flags)
{
    const element_token* tok;
    GET_TOKEN(tokeniser, *tindex, tok);

    // keep track of previous flags so we can check there are no duplicates
    std::vector<element_ast_flags> qualifier_flags;

    while (tok->type == ELEMENT_TOK_IDENTIFIER)
    {
        std::string id = tokeniser->text(tok);
        if (id == "intrinsic")
        {
            bool found_duplicate_intrinsic = false;
            for (element_ast_flags flag : qualifier_flags)
            {
                if (flag == ELEMENT_AST_FLAG_DECL_INTRINSIC)
                    found_duplicate_intrinsic = true;
            }

            //rather than log an error in here, let's assume the next thing to handle this token will give a useful error message
            if (found_duplicate_intrinsic)
                break;

            *flags |= ELEMENT_AST_FLAG_DECL_INTRINSIC;
            qualifier_flags.push_back(ELEMENT_AST_FLAG_DECL_INTRINSIC);
        }
        else
        {
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
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_declaration_invalid_identifier,
            ast->identifier);
    }

    bool function_declaration = false;
    if (ast->parent->type == ELEMENT_AST_NODE_FUNCTION)
        function_declaration = true;

    //If a function declaration identifier in another function or lambdas scope is "return" then that's valid, otherwise not
    const bool in_function_scope = ast->parent ? ast->parent->in_function_scope() : false;
    const bool in_lambda_scope = ast->parent ? ast->parent->in_lambda_scope() : false;
    const auto allow_reserved_names = function_declaration && (in_function_scope || in_lambda_scope);
    ELEMENT_OK_OR_RETURN(parse_identifier(tindex, ast, false, allow_reserved_names));

    GET_TOKEN(tokeniser, *tindex, tok);
    // always create the args node, even if it ends up being none/empty
    element_ast* args = ast->new_child();
    args->nearest_token = tok;
    if (tok->type == ELEMENT_TOK_BRACKETL)
    {
        // argument list
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_portlist(tindex, args));

        GET_TOKEN(tokeniser, *tindex, tok);
        if (tok->type != ELEMENT_TOK_BRACKETR)
        {
            return log_error(
                logger.get(),
                src_context.get(),
                tok,
                element::log_error_message_code::parse_declaration_missing_portlist_closing_parenthesis,
                tokeniser->text(tok),
                ast->identifier);
        }

        TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    }
    else
    {
        args->flags |= ELEMENT_AST_FLAG_DECL_EMPTY_INPUT;
    }

    const auto has_return = tok->type == ELEMENT_TOK_COLON;
    if (has_return)
    {
        if (find_return_type)
        {
            // output type
            tokenlist_advance(tokeniser, tindex);
            element_ast* type = ast->new_child();
            type->nearest_token = tok;
            ELEMENT_OK_OR_RETURN(parse_typename(tindex, type));
        }
        else
        {
            return log_error(
                logger.get(),
                src_context.get(),
                ast,
                element::log_error_message_code::parse_declaration_invalid_struct_return_type,
                ast->identifier);
        }
    }
    else
    {
        // implied any output
        element_ast* ret = ast->new_child(ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        ret->nearest_token = tok;
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
    while (token->type != ELEMENT_TOK_BRACER)
    {
        element_ast* item = ast->new_child();
        item->nearest_token = token;
        ELEMENT_OK_OR_RETURN(parse_item(tindex, item));
        GET_TOKEN(tokeniser, *tindex, token);
    }
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_anonymous_block(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    if (token->type == ELEMENT_TOK_BRACEL)
        TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, token);

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_ANONYMOUS_BLOCK;

    while (token->type != ELEMENT_TOK_BRACER)
    {
        element_ast* item = ast->new_child();
        item->nearest_token = token;
        ELEMENT_OK_OR_RETURN(parse_item(tindex, item));

        GET_TOKEN(tokeniser, *tindex, token);
        if (token->type != ELEMENT_TOK_BRACER && token->type != ELEMENT_TOK_COMMA)
            return ELEMENT_ERROR_MISSING_COMMA_IN_ANONYMOUS_BLOCK;

        if (token->type == ELEMENT_TOK_COMMA)
        {
            tokenlist_advance(tokeniser, tindex);
            GET_TOKEN(tokeniser, *tindex, token);
        }
    }
    tokenlist_advance(tokeniser, tindex);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_body(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    if (token->type == ELEMENT_TOK_BRACEL)
    {
        // scope (function body)
        ELEMENT_OK_OR_RETURN(parse_scope(tindex, ast));
    }
    else if (token->type == ELEMENT_TOK_EQUALS)
    {
        tokenlist_advance(tokeniser, tindex);
        ELEMENT_OK_OR_RETURN(parse_expression(tindex, ast));
    }
    else
    {
        if (ast->parent->type == ELEMENT_AST_NODE_FUNCTION)
        {
            return log_error(
                logger.get(),
                src_context.get(),
                token,
                element::log_error_message_code::parse_body_missing_body_for_function,
                ast->parent->children[ast_idx::function::declaration]->identifier,
                tokeniser->text(token));
        }

        return log_error(
            logger.get(),
            src_context.get(),
            token,
            element::log_error_message_code::parse_body_missing_body,
            ast->parent->children[ast_idx::function::declaration]->identifier,
            tokeniser->text(token));
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_function(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_FUNCTION;
    element_ast* const declaration = ast->new_child();
    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, declaration, true));
    declaration->flags = declflags;

    auto* body_node = ast->new_child();
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    body_node->nearest_token = body;
    if (body->type == ELEMENT_TOK_BRACEL || body->type == ELEMENT_TOK_EQUALS)
    {
        ELEMENT_OK_OR_RETURN(parse_body(tindex, body_node));
    }
    else
    {
        body_node->type = ELEMENT_AST_NODE_NO_BODY;
        if (declaration->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC))
        {
            //tokenlist_advance(tokeniser, tindex);
        }
        else
        {
            return log_error(
                logger.get(),
                src_context.get(),
                ast,
                element::log_error_message_code::parse_function_missing_body,
                declaration->identifier);
        }
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_struct(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    if (token->type != ELEMENT_TOK_IDENTIFIER)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_struct_missing_identifier,
            tokeniser->text(token));
    }

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_STRUCT;
    element_ast* declaration = ast->new_child();
    declaration->flags = declflags;

    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, declaration, false))

    const auto is_intrinsic = declaration->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    const auto has_portlist = !declaration->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);

    //todo: ask craig
    if (!is_intrinsic && !has_portlist)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_struct_nonintrinsic_missing_portlist,
            tokeniser->text(ast->nearest_token));
    }

    element_ast* body_node = ast->new_child();
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    body_node->nearest_token = body;
    //constraint, we have to assume this with no terminator, the next parsed statement will fail if syntax is incorrect
    body_node->type = ELEMENT_AST_NODE_NO_BODY;

    if (body->type == ELEMENT_TOK_BRACEL)
    {
        // scope (struct body)
        ELEMENT_OK_OR_RETURN(parse_scope(tindex, body_node));
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_constraint(size_t* tindex, element_ast* ast, element_ast_flags declflags)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);

    if (token->type == ELEMENT_TOK_EQUALS)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_constraint_invalid_identifier,
            tokeniser->text(token));
    }

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_CONSTRAINT;
    auto* declaration = ast->new_child();
    declaration->flags = declflags;

    // constraints can have return types
    ELEMENT_OK_OR_RETURN(parse_declaration(tindex, declaration, true))

    const auto is_intrinsic = declaration->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    const auto has_portlist = !declaration->children[0]->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);

    //todo: ask craig, port list for struct
    if (!is_intrinsic && !has_portlist)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_constraint_nonintrinsic_missing_portlist,
            tokeniser->text(ast->nearest_token));
    }

    element_ast* body_node = ast->new_child();
    const element_token* body;
    GET_TOKEN(tokeniser, *tindex, body);
    body_node->nearest_token = body;
    //tokenlist_advance(tokeniser, tindex);

    if (body->type == ELEMENT_TOK_BRACEL)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_constraint_has_body,
            ast->identifier);
    }
    else
    {
        body_node->type = ELEMENT_AST_NODE_NO_BODY;
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

    element_ast* scope = ast->new_child();
    ELEMENT_OK_OR_RETURN(parse_scope(tindex, scope));

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_item(size_t* tindex, element_ast* ast)
{
    const element_token* token;
    GET_TOKEN(tokeniser, *tindex, token);
    ast->nearest_token = token;

    // either a qualifier, 'struct', 'namespace' or a name; either way...
    if (token->type != ELEMENT_TOK_IDENTIFIER)
    {
        log(ELEMENT_ERROR_INVALID_IDENTIFIER,
            fmt::format("expected identifier, but found '{}' instead.", tokeniser->text(token)),
            ast);
        return ELEMENT_ERROR_INVALID_IDENTIFIER;
    }

    if (tokeniser->text(token) == "namespace")
    {
        ELEMENT_OK_OR_RETURN(parse_namespace(tindex, ast));
    }
    else
    {
        element_ast_flags flags = 0;
        // parse qualifiers
        ELEMENT_OK_OR_RETURN(parse_qualifiers(tindex, &flags));
        GET_TOKEN(tokeniser, *tindex, token);
        ast->nearest_token = token;
        if (tokeniser->text(token) == "struct")
        {
            tokenlist_advance(tokeniser, tindex);
            ELEMENT_OK_OR_RETURN(parse_struct(tindex, ast, flags));
        }
        else if (tokeniser->text(token) == "constraint")
        {
            tokenlist_advance(tokeniser, tindex);
            ELEMENT_OK_OR_RETURN(parse_constraint(tindex, ast, flags));
        }
        else
        {
            //consume "function" token ONLY if "intrinsic" qualifier precedes it
            if (tokeniser->text(token) == "function" && (flags & ELEMENT_AST_FLAG_DECL_INTRINSIC) == ELEMENT_AST_FLAG_DECL_INTRINSIC)
                tokenlist_advance(tokeniser, tindex);

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
    while (*tindex < tcount)
    {

        GET_TOKEN(tokeniser, *tindex, tok);
        if (tok->type == ELEMENT_TOK_EOF)
            return ELEMENT_OK;

        element_ast* item = ast->new_child();
        ELEMENT_OK_OR_RETURN(parse_item(tindex, item));
        if (*tindex < tcount && tok->type == ELEMENT_TOK_NONE)
            TOKENLIST_ADVANCE_AND_UPDATE(tokeniser, tindex, tok);
    }
    return ELEMENT_OK;
}

element_result element_parser_ctx::ast_build()
{
    // don't use ast_new here, as we need to return this pointer to the user
    element_ast_delete(&root);
    root = new element_ast(nullptr);
    size_t index = 0;
    auto result = parse(&index, root);
    if (result != ELEMENT_OK)
    {
        element_ast_delete(&root);
        root = nullptr;
        return result;
    }

    result = validate(root);
    return result;
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
    default:
        break;
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::validate_portlist(element_ast* ast)
{
    const auto length = ast->children.size();
    if (length == 0)
    {
        log(ELEMENT_ERROR_MISSING_PORTS, "portlist cannot be empty", ast);
        return ELEMENT_ERROR_MISSING_PORTS;
    }

    // ensure port identifiers are all unique
    element_result result = ELEMENT_OK;
    for (unsigned int i = 0; i < ast->children.size(); ++i)
    {
        for (unsigned int j = i; j < ast->children.size(); ++j)
        {
            if (i != j && // not the same port
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
    for (auto& child : ast->children)
    {

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

void element_parser_ctx::log(element_result message_code, const std::string& message, const element_ast* nearest_ast) const
{
    if (logger == nullptr)
        return;

    logger->log(*this, message_code, message, nearest_ast);
}

void element_parser_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return;

    logger->log(message, element_stage::ELEMENT_STAGE_MISC);
}
