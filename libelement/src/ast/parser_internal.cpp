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

static const std::unordered_set<std::string> qualifiers{ "intrinsic" };
static const std::unordered_set<std::string> constructs{ "struct", "namespace", "constraint" };
static const std::unordered_set<std::string> reserved_args{};
static const std::unordered_set<std::string> reserved_names{ "return" };

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

element_result element_parser_ctx::parse_literal(element_ast& terminal)
{
    assert(current_token->type == ELEMENT_TOK_NUMBER);
    terminal.type = ELEMENT_AST_NODE_LITERAL;
    terminal.literal = std::stof(tokeniser->text(current_token));
    return advance();
}

element_result element_parser_ctx::parse_identifier(element_ast& terminal, bool allow_reserved_args, bool allow_reserved_names)
{
    terminal.identifier.assign(tokeniser->text(current_token));

    if (current_token->type != ELEMENT_TOK_IDENTIFIER)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            &terminal,
            element::log_error_message_code::parse_identifier_failed,
            terminal.identifier);
    }

    ELEMENT_OK_OR_RETURN(advance());

    const auto result = check_reserved_words(terminal.identifier, allow_reserved_args, allow_reserved_names);
    if (result != ELEMENT_OK)
    {
        return log_error(
            logger.get(),
            src_context.get(),
            &terminal,
            element::log_error_message_code::parse_identifier_reserved,
            terminal.identifier);
    }

    return result;
}

element_result element_parser_ctx::parse_typename(element_ast& parent)
{
    auto& typename_ast = make_node(parent, current_token, ELEMENT_AST_NODE_TYPENAME);

    const element_result result = parse_expression(typename_ast);
    if (result != ELEMENT_OK)
    {
        //todo: change error from identifier to invalid expression
        return log_error(
            logger.get(),
            src_context.get(),
            &typename_ast,
            element::log_error_message_code::parse_typename_not_identifier,
            tokeniser->text(typename_ast.nearest_token));
    }

    return result;
}

element_result element_parser_ctx::parse_port(element_ast& parent)
{
    auto& port = make_node(parent, current_token, ELEMENT_AST_NODE_PORT);

    if (current_token->type == ELEMENT_TOK_IDENTIFIER)
    {
        ELEMENT_OK_OR_RETURN(parse_identifier(port, true));
    }
    else if (current_token->type == ELEMENT_TOK_UNDERSCORE)
    {
        // no name, advance
        ELEMENT_OK_OR_RETURN(advance());
    }
    else
    {
        return log_error(
            logger.get(),
            src_context.get(),
            &port,
            element::log_error_message_code::parse_port_failed,
            tokeniser->text(current_token));
    }

    if (current_token->type == ELEMENT_TOK_COLON)
    {
        ELEMENT_OK_OR_RETURN(advance());
        ELEMENT_OK_OR_RETURN(parse_typename(port));
    }
    else
    {
        make_node(port, current_token, ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
    }

    if (current_token->type == ELEMENT_TOK_EQUALS)
    {
        ELEMENT_OK_OR_RETURN(advance());
        ELEMENT_OK_OR_RETURN(parse_expression(port));
    }
    else
    {
        make_node(port, current_token, ELEMENT_AST_NODE_UNSPECIFIED_DEFAULT);
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_portlist(element_ast& parent)
{
    //the previous token is the bracket
    auto& port_ast = make_node(parent, previous_token, ELEMENT_AST_NODE_PORTLIST);

    do
    {
        ELEMENT_OK_OR_RETURN(parse_port(port_ast));
    } while (current_token->type == ELEMENT_TOK_COMMA && advance() == ELEMENT_OK);

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_exprlist(element_ast& parent)
{
    //the caller should ensure it's a '('
    assert(current_token->type == ELEMENT_TOK_BRACKETL);

    ELEMENT_OK_OR_RETURN(advance());
    auto& exprlist = make_node(parent, current_token, ELEMENT_AST_NODE_EXPRLIST);

    //should be '(' for previous and ')' for current
    if (current_token->type == ELEMENT_TOK_BRACKETR)
        return log_error(logger.get(),
            build_source_info(src_context.get(), previous_token, current_token->tok_len),
            element::log_error_message_code::parse_exprlist_empty,
            parent.identifier);

    do
    {
        ELEMENT_OK_OR_RETURN(parse_expression(exprlist));
    } while (current_token->type == ELEMENT_TOK_COMMA && advance() == ELEMENT_OK);

    if (current_token->type != ELEMENT_TOK_BRACKETR)
        return log_error(logger.get(),
            src_context.get(),
            current_token,
            element::log_error_message_code::parse_exprlist_missing_closing_parenthesis,
            parent.identifier,
            tokeniser->text(current_token));

    return advance();
}

element_result element_parser_ctx::parse_call(element_ast& parent)
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

    auto& root_call = make_node(parent, current_token, ELEMENT_AST_NODE_CALL);
    if (current_token->type == ELEMENT_TOK_IDENTIFIER)
    {
        ELEMENT_OK_OR_RETURN(parse_identifier(root_call));
    }
    else if (current_token->type == ELEMENT_TOK_NUMBER)
    {
        ELEMENT_OK_OR_RETURN(parse_literal(root_call));
    }
    else
    {
        return log_error(
            logger.get(),
            src_context.get(),
            current_token,
            element::log_error_message_code::parse_call_invalid_expression,
            parent.identifier,
            tokeniser->text(current_token));
    }

    while (current_token->type == ELEMENT_TOK_DOT || current_token->type == ELEMENT_TOK_BRACKETL)
    {
        if (current_token->type == ELEMENT_TOK_BRACKETL)
        {
            // call with args
            // TODO: bomb out if we're trying to call a literal, keep track of previous node
            ELEMENT_OK_OR_RETURN(parse_exprlist(root_call));
        }
        else if (current_token->type == ELEMENT_TOK_DOT)
        {
            //advance over the dot so we're now at (what should be) the identifier token
            ELEMENT_OK_OR_RETURN(advance());
            auto& call = make_node(root_call, current_token, ELEMENT_AST_NODE_CALL);
            ELEMENT_OK_OR_RETURN(parse_identifier(call));
        }
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_lambda(element_ast& parent)
{
    //the caller should ensure it's an underscore
    assert(current_token->type == ELEMENT_TOK_UNDERSCORE);

    auto& lambda = make_node(parent, current_token, ELEMENT_AST_NODE_LAMBDA);

    ELEMENT_OK_OR_RETURN(advance_then_check(ELEMENT_TOK_BRACKETL, ELEMENT_ERROR_PARSE));
    ELEMENT_OK_OR_RETURN(advance());
    ELEMENT_OK_OR_RETURN(parse_portlist(lambda));

    //todo: logging
    if (current_token->type != ELEMENT_TOK_BRACKETR)
        return ELEMENT_ERROR_PARSE;

    ELEMENT_OK_OR_RETURN(advance());

    const auto has_return = current_token->type == ELEMENT_TOK_COLON;
    if (has_return)
    {
        ELEMENT_OK_OR_RETURN(advance());
        ELEMENT_OK_OR_RETURN(parse_typename(lambda));
    }
    else
    {
        make_node(lambda, current_token, ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
    }

    return parse_function_body(lambda);
}

element_result element_parser_ctx::parse_expression(element_ast& parent)
{
    if (current_token->type == ELEMENT_TOK_IDENTIFIER || current_token->type == ELEMENT_TOK_NUMBER)
        return parse_call(parent);

    if (current_token->type == ELEMENT_TOK_UNDERSCORE)
        return parse_lambda(parent);

    if (current_token->type == ELEMENT_TOK_BRACEL)
        return parse_anonymous_block(parent);

    return log_error(
        logger.get(),
        src_context.get(),
        &parent,
        element::log_error_message_code::parse_expression_failed,
        tokeniser->text(current_token));
}

element_result element_parser_ctx::parse_qualifiers(element_ast_flags& flags)
{
    // keep track of previous flags so we can check there are no duplicates
    std::vector<element_ast_flags> qualifier_flags;

    while (current_token->type == ELEMENT_TOK_IDENTIFIER)
    {
        std::string id = tokeniser->text(current_token);
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

            flags |= ELEMENT_AST_FLAG_DECL_INTRINSIC;
            qualifier_flags.push_back(ELEMENT_AST_FLAG_DECL_INTRINSIC);
        }
        else
        {
            break;
        }

        ELEMENT_OK_OR_RETURN(advance());
    }

    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_declaration(element_ast& parent, element_ast_flags flags)
{
    //If a function declaration identifier in another function or lambdas scope is "return" then that's valid, otherwise not
    const bool in_scope = parent.in_function_scope() || parent.in_lambda_scope();
    const bool function_declaration = parent.type == ELEMENT_AST_NODE_FUNCTION;
    const bool allow_reserved_names = function_declaration && in_scope;

    auto& declaration = make_node(parent, current_token, ELEMENT_AST_NODE_DECLARATION, flags);
    ELEMENT_OK_OR_RETURN(parse_identifier(declaration, false, allow_reserved_names));

    if (current_token->type == ELEMENT_TOK_BRACKETL)
    {
        ELEMENT_OK_OR_RETURN(advance());
        ELEMENT_OK_OR_RETURN(parse_portlist(declaration));

        if (current_token->type != ELEMENT_TOK_BRACKETR)
        {
            return log_error(
                logger.get(),
                src_context.get(),
                current_token,
                element::log_error_message_code::parse_declaration_missing_portlist_closing_parenthesis,
                tokeniser->text(current_token),
                declaration.identifier);
        }

        ELEMENT_OK_OR_RETURN(advance());
    }
    else
    {
        make_node(declaration, current_token, ELEMENT_AST_NODE_NONE, ELEMENT_AST_FLAG_DECL_EMPTY_INPUT);
    }

    const bool has_return = current_token->type == ELEMENT_TOK_COLON;
    if (has_return)
    {
        ELEMENT_OK_OR_RETURN(advance());
        return parse_typename(declaration);
    }

    // implied any output
    make_node(declaration, current_token, ELEMENT_AST_NODE_UNSPECIFIED_TYPE);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_scope(element_ast& parent)
{
    auto& scope = make_node(parent, current_token, ELEMENT_AST_NODE_SCOPE);

    if (current_token->type == ELEMENT_TOK_BRACEL)
        ELEMENT_OK_OR_RETURN(advance());

    while (current_token->type != ELEMENT_TOK_BRACER)
        ELEMENT_OK_OR_RETURN(parse_item(scope));

    return advance();
}

element_result element_parser_ctx::parse_anonymous_block(element_ast& parent)
{
    assert(current_token->type == ELEMENT_TOK_BRACEL);
    ELEMENT_OK_OR_RETURN(advance_then_check_not(ELEMENT_TOK_BRACER, ELEMENT_ERROR_PARSE));

    auto& block = make_node(parent, current_token, ELEMENT_AST_NODE_ANONYMOUS_BLOCK);

    while (current_token->type != ELEMENT_TOK_BRACER)
    {
        ELEMENT_OK_OR_RETURN(parse_item(block));

        if (current_token->type != ELEMENT_TOK_BRACER && current_token->type != ELEMENT_TOK_COMMA)
            return ELEMENT_ERROR_MISSING_COMMA_IN_ANONYMOUS_BLOCK;
        
        if (current_token->type == ELEMENT_TOK_COMMA)
            ELEMENT_OK_OR_RETURN(advance());
    }

    return advance();
}

element_result element_parser_ctx::parse_function_body(element_ast& parent)
{    
    if (current_token->type == ELEMENT_TOK_BRACEL)
        return parse_scope(parent);

    if (current_token->type == ELEMENT_TOK_EQUALS)
    {
        ELEMENT_OK_OR_RETURN(advance());
        return parse_expression(parent);
    }

    const element::log_error_message_code code = parent.parent->type == ELEMENT_AST_NODE_FUNCTION
                                                     ? element::log_error_message_code::parse_body_missing_body_for_function
                                                     : element::log_error_message_code::parse_body_missing_body;

    return log_error(
        logger.get(),
        src_context.get(),
        current_token,
        code,
        parent.parent->children[ast_idx::function::declaration]->identifier,
        tokeniser->text(current_token));
}

element_result element_parser_ctx::parse_function(element_ast& parent, element_ast_flags declflags)
{
    //consume "function" token ONLY if "intrinsic" qualifier precedes it
    if (tokeniser->text(current_token) == "function" && (declflags & ELEMENT_AST_FLAG_DECL_INTRINSIC) == ELEMENT_AST_FLAG_DECL_INTRINSIC)
        ELEMENT_OK_OR_RETURN(advance());

    auto& func = make_node(parent, current_token, ELEMENT_AST_NODE_FUNCTION);
    ELEMENT_OK_OR_RETURN(parse_declaration(func, declflags));
    const auto* declaration = func.function_get_declaration();

    if (current_token->type == ELEMENT_TOK_BRACEL || current_token->type == ELEMENT_TOK_EQUALS)
        return parse_function_body(func);

    make_node(func, current_token, ELEMENT_AST_NODE_NO_BODY);
    if (declaration->declaration_is_intrinsic())
        return ELEMENT_OK;

    return log_error(
        logger.get(),
        src_context.get(),
        declaration,
        element::log_error_message_code::parse_function_missing_body,
        declaration->identifier);
}

element_result element_parser_ctx::parse_struct(element_ast& parent, element_ast_flags declflags)
{
    ELEMENT_OK_OR_RETURN(advance());
    auto& struct_node = make_node(parent, current_token, ELEMENT_AST_NODE_STRUCT);
    ELEMENT_OK_OR_RETURN(parse_declaration(struct_node, declflags));
    const auto* declaration = struct_node.struct_get_declaration();

    if (!declaration->declaration_is_intrinsic() && !declaration->declaration_has_portlist())
    {
        return log_error(
            logger.get(),
            src_context.get(),
            declaration,
            element::log_error_message_code::parse_struct_nonintrinsic_missing_portlist,
            declaration->identifier);
    }
    
    if (declaration->declaration_has_outputs())
    {
        return log_error(
            logger.get(),
            src_context.get(),
            declaration,
            element::log_error_message_code::parse_declaration_invalid_struct_return_type,
            declaration->identifier);
    }

    if (current_token->type == ELEMENT_TOK_BRACEL)
        return parse_scope(struct_node);

    make_node(struct_node, current_token, ELEMENT_AST_NODE_NO_BODY);
    return ELEMENT_OK;
}

element_result element_parser_ctx::parse_constraint(element_ast& parent, element_ast_flags declflags)
{
    ELEMENT_OK_OR_RETURN(advance());
    auto& constraint = make_node(parent, current_token, ELEMENT_AST_NODE_CONSTRAINT);
    ELEMENT_OK_OR_RETURN(parse_declaration(constraint, declflags));
    const auto* declaration = constraint.constraint_get_declaration();

    make_node(constraint, current_token, ELEMENT_AST_NODE_NO_BODY);

    if (!declaration->declaration_is_intrinsic() && !declaration->declaration_has_portlist())
    {
        return log_error(
            logger.get(),
            src_context.get(),
            declaration,
            element::log_error_message_code::parse_constraint_nonintrinsic_missing_portlist,
            declaration->identifier);
    }

    if (current_token->type == ELEMENT_TOK_BRACEL)
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

element_result element_parser_ctx::parse_namespace(element_ast& parent)
{
    ELEMENT_OK_OR_RETURN(advance());
    auto& namespace_node = make_node(parent, current_token, ELEMENT_AST_NODE_NAMESPACE);
    ELEMENT_OK_OR_RETURN(parse_identifier(namespace_node));
    return parse_scope(namespace_node);
}

element_result element_parser_ctx::parse_item(element_ast& parent)
{
    if (tokeniser->text(current_token) == "namespace")
        return parse_namespace(parent);

    element_ast_flags flags = 0;
    ELEMENT_OK_OR_RETURN(parse_qualifiers(flags));

    if (tokeniser->text(current_token) == "struct")
        return parse_struct(parent, flags);

    if (tokeniser->text(current_token) == "constraint")
        return parse_constraint(parent, flags);

    return parse_function(parent, flags);
}

element_result element_parser_ctx::parse(std::size_t& tindex, element_ast* input_ast)
{
    element_result result = ELEMENT_OK;
    current_token = tokeniser->get_token(tindex, result);
    if (result != ELEMENT_OK)
        return result;

    input_ast->nearest_token = current_token;
    input_ast->type = ELEMENT_AST_NODE_ROOT;

    if (static_cast<int>(tindex) < static_cast<int>(tokeniser->tokens.size()) - 1 && current_token->type == ELEMENT_TOK_NONE)
        ELEMENT_OK_OR_RETURN(advance());

    while (tindex < tokeniser->tokens.size())
    {
        if (current_token->type == ELEMENT_TOK_EOF)
            return ELEMENT_OK;

        ELEMENT_OK_OR_RETURN(parse_item(*input_ast));
    }

    assert(!"no EOF token");
    return ELEMENT_ERROR_UNKNOWN;
}

element_result element_parser_ctx::ast_build()
{
    // don't use ast_new here, as we need to return this pointer to the user
    element_ast_delete(&root);
    root = new element_ast(nullptr);
    size_t index = 0;
    const auto result = parse(index, root);
    if (result != ELEMENT_OK)
    {
        element_ast_delete(&root);
        return result;
    }

    return validate(root);
}

element_result element_parser_ctx::advance()
{
    element_result result = ELEMENT_OK;
    previous_token = current_token;

    // TODO: do something with NONE tokens, we might need them later to preserve formatting...
    do
    {
        token_index++;
        current_token = tokeniser->get_token(token_index, result);
    } while (token_index < tokeniser->tokens.size() - 1 && current_token && current_token->type == ELEMENT_TOK_NONE);

    return result;
}

element_result element_parser_ctx::advance_then_check(element_token_type type, element_result result_on_failed_match)
{
    ELEMENT_OK_OR_RETURN(advance());

    //todo: logging
    if (current_token->type != type)
        return result_on_failed_match;

    return ELEMENT_OK;
}

element_result element_parser_ctx::advance_then_check_not(element_token_type type, element_result result_on_successful_match)
{
    ELEMENT_OK_OR_RETURN(advance());

    //todo: logging
    if (current_token->type == type)
        return result_on_successful_match;

    return ELEMENT_OK;
}

element_ast& element_parser_ctx::make_node(element_ast& parent, element_token* token, element_ast_node_type type, element_ast_flags flags) const
{
    auto* ast = parent.new_child(type);
    ast->nearest_token = token;
    ast->flags = flags;
    return *ast;
}

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

element_result element_parser_ctx::log(element_result message_code, const std::string& message, const element_ast* nearest_ast) const
{
    if (logger == nullptr)
        return message_code;

    logger->log(*this, message_code, message, nearest_ast);
    return message_code;
}

element_result element_parser_ctx::log(const std::string& message) const
{
    return log(ELEMENT_OK, message, nullptr);
}