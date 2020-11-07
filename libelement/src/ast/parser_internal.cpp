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

element_result element_parser_ctx::parse_literal()
{
    assert(token->type == ELEMENT_TOK_NUMBER);
    ast->type = ELEMENT_AST_NODE_LITERAL;
    ast->literal = std::stof(tokeniser->text(token));
    ELEMENT_OK_OR_RETURN(next_token());
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_identifier(bool allow_reserved_args, bool allow_reserved_names)
{
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

    ELEMENT_OK_OR_RETURN(next_token());

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

element_result element_parser_ctx::parse_typename()
{
    auto* typename_ast = ast;
    auto* typename_token = token;
    typename_ast->nearest_token = token;
    typename_ast->type = ELEMENT_AST_NODE_TYPENAME;

    ast = typename_ast->new_child();
    const element_result result = parse_expression();
    if (result != ELEMENT_OK)
    {
        //todo: change error from identifier to invalid expression
        return log_error(
            logger.get(),
            src_context.get(),
            typename_ast,
            element::log_error_message_code::parse_typename_not_identifier,
            tokeniser->text(typename_token));
    }

    return result;
}

element_result element_parser_ctx::parse_port()
{
    auto* port_ast = ast;
    port_ast->nearest_token = token;
    port_ast->type = ELEMENT_AST_NODE_PORT;

    if (token->type == ELEMENT_TOK_IDENTIFIER)
    {
        ELEMENT_OK_OR_RETURN(parse_identifier(true));
    }
    else if (token->type == ELEMENT_TOK_UNDERSCORE)
    {
        // no name, advance
        ELEMENT_OK_OR_RETURN(next_token());
    }
    else
    {
        return log_error(
            logger.get(),
            src_context.get(),
            ast,
            element::log_error_message_code::parse_port_failed,
            tokeniser->text(token));
    }

    ast = port_ast->new_child(ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
    ast->nearest_token = token;
    if (token->type == ELEMENT_TOK_COLON)
    {
        ELEMENT_OK_OR_RETURN(next_token());
        ELEMENT_OK_OR_RETURN(parse_typename());
    }

    ast = port_ast->new_child(ELEMENT_AST_NODE_UNSPECIFIED_DEFAULT);
    ast->nearest_token = token;
    if (token->type == ELEMENT_TOK_EQUALS)
    {
        ELEMENT_OK_OR_RETURN(next_token());
        ELEMENT_OK_OR_RETURN(parse_expression());
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_portlist()
{
    //the previous token is the bracket
    element_result result = ELEMENT_OK;
    ast->nearest_token = tokeniser->get_token(token_index - 1, result);
    if (result != ELEMENT_OK)
        return result;

    ast->type = ELEMENT_AST_NODE_PORTLIST;
    ast->flags = 0;

    auto* port_ast = ast;

    do
    {
        ast = port_ast->new_child();
        ELEMENT_OK_OR_RETURN(parse_port());
    } while (token->type == ELEMENT_TOK_COMMA && next_token() == ELEMENT_OK);

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_exprlist()
{
    //the caller should ensure it's a '('
    const auto* previous_token = token;
    assert(previous_token->type == ELEMENT_TOK_BRACKETL);

    ELEMENT_OK_OR_RETURN(next_token());
    auto* exprlist_ast = ast;
    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_EXPRLIST;

    if (token->type != ELEMENT_TOK_BRACKETR)
    {
        do
        {
            ast = exprlist_ast->new_child();
            ast->nearest_token = token;
            ELEMENT_OK_OR_RETURN(parse_expression());
        } while (token->type == ELEMENT_TOK_COMMA && next_token() == ELEMENT_OK);
    }
    else
    {
        //should be '(' for previous and ')' for current
        auto info = build_source_info(src_context.get(), previous_token, token->tok_len);

        return log_error(
            logger.get(),
            info,
            element::log_error_message_code::parse_exprlist_empty,
            exprlist_ast->parent->identifier);
    }

    if (token->type != ELEMENT_TOK_BRACKETR)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            token,
            element::log_error_message_code::parse_exprlist_missing_closing_parenthesis,
            exprlist_ast->parent->identifier,
            tokeniser->text(token));
    }

    ELEMENT_OK_OR_RETURN(next_token());
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_call()
{
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

    element_ast* root_call = ast;
    if (token->type == ELEMENT_TOK_IDENTIFIER)
    {
        // get identifier
        ELEMENT_OK_OR_RETURN(parse_identifier());
        root_call->type = ELEMENT_AST_NODE_CALL;
    }
    else if (token->type == ELEMENT_TOK_NUMBER)
    {
        // this will change root's type to LITERAL
        ELEMENT_OK_OR_RETURN(parse_literal());
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

    while (token->type == ELEMENT_TOK_DOT || token->type == ELEMENT_TOK_BRACKETL)
    {
        if (token->type == ELEMENT_TOK_BRACKETL)
        {
            // call with args
            // TODO: bomb out if we're trying to call a literal, keep track of previous node

            ast = root_call->new_child();
            ELEMENT_OK_OR_RETURN(parse_exprlist());
        }
        else if (token->type == ELEMENT_TOK_DOT)
        {
            //advance over the dot so we're now at (what should be) the identifier token
            ELEMENT_OK_OR_RETURN(next_token());

            ast = root_call->new_child(ELEMENT_AST_NODE_CALL);
            ELEMENT_OK_OR_RETURN(parse_identifier());
        }
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_lambda()
{
    //the caller should ensure it's an underscore
    assert(token->type == ELEMENT_TOK_UNDERSCORE);

    auto* lambda_ast = ast;
    lambda_ast->nearest_token = token;
    lambda_ast->type = ELEMENT_AST_NODE_LAMBDA;

    ELEMENT_OK_OR_RETURN(next_token());

    //todo: logging
    if (token->type != ELEMENT_TOK_BRACKETL)
        return ELEMENT_ERROR_INVALID_OPERATION;

    ELEMENT_OK_OR_RETURN(next_token());

    element_ast* ports = lambda_ast->new_child();
    ast = ports;
    ELEMENT_OK_OR_RETURN(parse_portlist());

    //todo: logging
    if (token->type != ELEMENT_TOK_BRACKETR)
        return ELEMENT_ERROR_INVALID_OPERATION;

    ELEMENT_OK_OR_RETURN(next_token());

    element_ast* type = lambda_ast->new_child();
    ast = type;
    type->nearest_token = token;
    const auto has_return = token->type == ELEMENT_TOK_COLON;
    if (has_return)
    {
        ELEMENT_OK_OR_RETURN(next_token());
        return parse_typename();
    }

    type->type = ELEMENT_AST_NODE_UNSPECIFIED_TYPE;
    return parse_function_body(lambda_ast);
}

element_result element_parser_ctx::parse_expression()
{
    ast->nearest_token = token;

    if (token->type == ELEMENT_TOK_IDENTIFIER || token->type == ELEMENT_TOK_NUMBER)
        return parse_call();

    if (token->type == ELEMENT_TOK_UNDERSCORE)
        return parse_lambda();

    if (token->type == ELEMENT_TOK_BRACEL)
        return parse_anonymous_block();

    return log_error(
        logger.get(),
        src_context.get(),
        ast,
        element::log_error_message_code::parse_expression_failed,
        tokeniser->text(token));
}

element_result element_parser_ctx::parse_qualifiers(element_ast_flags* flags)
{
    // keep track of previous flags so we can check there are no duplicates
    std::vector<element_ast_flags> qualifier_flags;

    while (token->type == ELEMENT_TOK_IDENTIFIER)
    {
        std::string id = tokeniser->text(token);
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

        ELEMENT_OK_OR_RETURN(next_token());
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_declaration(bool find_return_type)
{
    auto* decl_ast = ast;
    decl_ast->nearest_token = token;
    decl_ast->type = ELEMENT_AST_NODE_DECLARATION;

    if (token->type != ELEMENT_TOK_IDENTIFIER)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            decl_ast,
            element::log_error_message_code::parse_declaration_invalid_identifier,
            tokeniser->text(token));
    }

    bool function_declaration = false;
    if (decl_ast->parent->type == ELEMENT_AST_NODE_FUNCTION)
        function_declaration = true;

    //If a function declaration identifier in another function or lambdas scope is "return" then that's valid, otherwise not
    const bool in_function_scope = decl_ast->parent ? decl_ast->parent->in_function_scope() : false;
    const bool in_lambda_scope = decl_ast->parent ? decl_ast->parent->in_lambda_scope() : false;
    const auto allow_reserved_names = function_declaration && (in_function_scope || in_lambda_scope);
    ELEMENT_OK_OR_RETURN(parse_identifier(false, allow_reserved_names));

    // always create the args node, even if it ends up being none/empty
    element_ast* args = decl_ast->new_child();
    ast = args;
    args->nearest_token = token;
    if (token->type == ELEMENT_TOK_BRACKETL)
    {
        // argument list
        ELEMENT_OK_OR_RETURN(next_token());
        ELEMENT_OK_OR_RETURN(parse_portlist());

        if (token->type != ELEMENT_TOK_BRACKETR)
        {
            return log_error(
                logger.get(),
                src_context.get(),
                token,
                element::log_error_message_code::parse_declaration_missing_portlist_closing_parenthesis,
                tokeniser->text(token),
                decl_ast->identifier);
        }

        ELEMENT_OK_OR_RETURN(next_token());
    }
    else
    {
        args->flags |= ELEMENT_AST_FLAG_DECL_EMPTY_INPUT;
    }

    const auto has_return = token->type == ELEMENT_TOK_COLON;
    if (has_return)
    {
        if (find_return_type)
        {
            // output type
            ELEMENT_OK_OR_RETURN(next_token());
            element_ast* type = decl_ast->new_child();
            ast = type;
            type->nearest_token = token;
            ELEMENT_OK_OR_RETURN(parse_typename());
        }
        else
        {
            return log_error(
                logger.get(),
                src_context.get(),
                decl_ast,
                element::log_error_message_code::parse_declaration_invalid_struct_return_type,
                decl_ast->identifier);
        }
    }
    else
    {
        // implied any output
        element_ast* ret = decl_ast->new_child(ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
        ast = ret;
        ret->nearest_token = token;
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_scope(element_ast* parent)
{
    auto* our_ast = new_ast(parent, token, ELEMENT_AST_NODE_SCOPE);

    if (token->type == ELEMENT_TOK_BRACEL)
        ELEMENT_OK_OR_RETURN(next_token());

    while (token->type != ELEMENT_TOK_BRACER)
        ELEMENT_OK_OR_RETURN(parse_item(our_ast));

    return next_token();
}

element_result element_parser_ctx::parse_anonymous_block()
{
    auto* block_ast = ast;
    block_ast->nearest_token = token;
    block_ast->type = ELEMENT_AST_NODE_ANONYMOUS_BLOCK;

    if (token->type == ELEMENT_TOK_BRACEL)
        ELEMENT_OK_OR_RETURN(next_token());

    while (token->type != ELEMENT_TOK_BRACER)
    {
        ELEMENT_OK_OR_RETURN(parse_item(block_ast));

        if (token->type != ELEMENT_TOK_BRACER && token->type != ELEMENT_TOK_COMMA)
            return ELEMENT_ERROR_MISSING_COMMA_IN_ANONYMOUS_BLOCK;

        if (token->type == ELEMENT_TOK_COMMA)
            ELEMENT_OK_OR_RETURN(next_token());
    }

    ELEMENT_OK_OR_RETURN(next_token());
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_function_body(element_ast* parent)
{    
    //function body is a scope
    if (token->type == ELEMENT_TOK_BRACEL)
        return parse_scope(parent);

    if (token->type == ELEMENT_TOK_EQUALS)
    {
        auto* body_node = new_ast(parent, token, ELEMENT_AST_NODE_NO_BODY);
        ELEMENT_OK_OR_RETURN(next_token());
        return parse_expression();
    }

    const element::log_error_message_code code = ast->parent->type == ELEMENT_AST_NODE_FUNCTION
                                                     ? element::log_error_message_code::parse_body_missing_body_for_function
                                                     : element::log_error_message_code::parse_body_missing_body;

    return log_error(
        logger.get(),
        src_context.get(),
        token,
        code,
        ast->parent->children[ast_idx::function::declaration]->identifier,
        tokeniser->text(token));
}

element_result element_parser_ctx::parse_function(element_ast* parent, element_ast_flags declflags)
{
    //consume "function" token ONLY if "intrinsic" qualifier precedes it
    if (tokeniser->text(token) == "function" && (declflags & ELEMENT_AST_FLAG_DECL_INTRINSIC) == ELEMENT_AST_FLAG_DECL_INTRINSIC)
        ELEMENT_OK_OR_RETURN(next_token());

    auto* func_ast = new_ast(parent, token, ELEMENT_AST_NODE_FUNCTION);
    auto* const declaration = new_ast(func_ast, token, ELEMENT_AST_NODE_DECLARATION, declflags);
    ELEMENT_OK_OR_RETURN(parse_declaration(true));

    if (token->type == ELEMENT_TOK_BRACEL || token->type == ELEMENT_TOK_EQUALS)
        return parse_function_body(func_ast);

    new_ast(func_ast, token, ELEMENT_AST_NODE_NO_BODY);
    if (declaration->declaration_is_intrinsic())
        return ELEMENT_OK;

    return log_error(
        logger.get(),
        src_context.get(),
        declaration,
        element::log_error_message_code::parse_function_missing_body,
        declaration->identifier);
}

element_result element_parser_ctx::parse_struct(element_ast* parent, element_ast_flags declflags)
{
    ELEMENT_OK_OR_RETURN(next_token());
    auto* struct_ast = new_ast(parent, token, ELEMENT_AST_NODE_STRUCT);
    element_ast* declaration = new_ast(struct_ast, token, ELEMENT_AST_NODE_DECLARATION, declflags);
    ELEMENT_OK_OR_RETURN(parse_declaration(false));

    if (!declaration->declaration_is_intrinsic() && !declaration->declaration_has_portlist())
    {
        return log_error(
            logger.get(),
            src_context.get(),
            declaration,
            element::log_error_message_code::parse_struct_nonintrinsic_missing_portlist,
            declaration->identifier);
    }

    //struct body is a scope
    if (token->type == ELEMENT_TOK_BRACEL)
        return parse_scope(struct_ast);

    //there is no body
    new_ast(struct_ast, token, ELEMENT_AST_NODE_NO_BODY);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_constraint(element_ast* parent, element_ast_flags declflags)
{
    ELEMENT_OK_OR_RETURN(next_token());
    auto* constraint_ast = new_ast(parent, token, ELEMENT_AST_NODE_CONSTRAINT);
    auto* declaration = new_ast(constraint_ast, token, ELEMENT_AST_NODE_DECLARATION, declflags);
    ELEMENT_OK_OR_RETURN(parse_declaration(true));
    new_ast(constraint_ast, token, ELEMENT_AST_NODE_NO_BODY);

    if (!declaration->declaration_is_intrinsic() && !declaration->declaration_has_portlist())
    {
        return log_error(
            logger.get(),
            src_context.get(),
            declaration,
            element::log_error_message_code::parse_constraint_nonintrinsic_missing_portlist,
            declaration->identifier);
    }

    if (token->type == ELEMENT_TOK_BRACEL)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            declaration,
            element::log_error_message_code::parse_constraint_has_body,
            declaration->identifier);
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_namespace(element_ast* parent)
{
    auto* our_ast = new_ast(parent, token, ELEMENT_AST_NODE_NAMESPACE);
    ELEMENT_OK_OR_RETURN(next_token());
    ELEMENT_OK_OR_RETURN(parse_identifier());
    return parse_scope(our_ast);
}

element_result element_parser_ctx::parse_item(element_ast* parent)
{
    ast = parent;

    if (tokeniser->text(token) == "namespace")
        return parse_namespace(parent);

    element_ast_flags flags = 0;
    ELEMENT_OK_OR_RETURN(parse_qualifiers(&flags));

    if (tokeniser->text(token) == "struct")
        ELEMENT_OK_OR_RETURN(parse_struct(parent, flags));
    else if (tokeniser->text(token) == "constraint")
        ELEMENT_OK_OR_RETURN(parse_constraint(parent, flags));
    else
        ELEMENT_OK_OR_RETURN(parse_function(parent, flags));

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse(size_t* tindex, element_ast* input_ast)
{
    size_t tcount = tokeniser->tokens.size();

    element_result result = ELEMENT_OK;
    token = tokeniser->get_token(*tindex, result);
    if (result != ELEMENT_OK)
        return result;

    ast = input_ast;

    ast->nearest_token = token;
    ast->type = ELEMENT_AST_NODE_ROOT;

    if (*tindex < tcount && token->type == ELEMENT_TOK_NONE)
        ELEMENT_OK_OR_RETURN(next_token());

    while (*tindex < tcount)
    {
        /*token = tokeniser->get_token(*tindex, result);
        if (result != ELEMENT_OK)
            return result;*/

        if (token->type == ELEMENT_TOK_EOF)
            return ELEMENT_OK;

        ELEMENT_OK_OR_RETURN(parse_item(input_ast));
        if (*tindex < tcount && token->type == ELEMENT_TOK_NONE)
            ELEMENT_OK_OR_RETURN(next_token());
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

element_result element_parser_ctx::next_token()
{
    element_result result = ELEMENT_OK;

    // TODO: do something with NONE tokens, we might need them later to preserve formatting...
    do
    {
        token_index++;
        token = tokeniser->get_token(token_index, result);
    } while (token_index < tokeniser->tokens.size() - 1 && token && token->type == ELEMENT_TOK_NONE);

    return result;
}

element_ast* element_parser_ctx::new_ast(element_ast* parent, element_token* token, element_ast_node_type type, element_ast_flags flags)
{
    assert(parent);

    auto* ast = parent->new_child(type);
    this->ast = ast;
    ast->nearest_token = token;
    ast->flags = flags;
    return ast;
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

element_result element_parser_ctx::log(element_result message_code, const std::string& message, const element_ast* nearest_ast) const
{
    if (logger == nullptr)
        return message_code;

    logger->log(*this, message_code, message, nearest_ast);
    return message_code;
}

element_result element_parser_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return ELEMENT_OK;

    logger->log(message, element_stage::ELEMENT_STAGE_MISC);
    return ELEMENT_OK;
}
