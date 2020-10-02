#include "common_internal.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "token_internal.hpp"
#include "ast/ast_internal.hpp"
#include "configuration.hpp"
#include "etree/expressions.hpp"
#include "lmnt/compiler.hpp"

#define PRINTCASE(a) \
    case a:          \
        c = #a;      \
        break;
std::string tokens_to_string(const element_tokeniser_ctx* context, const element_token* nearest_token)
{
    std::string string;

    for (const auto& token : context->tokens)
    {
        std::string c;
        switch (token.type)
        {
            PRINTCASE(ELEMENT_TOK_NONE)
            PRINTCASE(ELEMENT_TOK_NUMBER)
            PRINTCASE(ELEMENT_TOK_IDENTIFIER)
            PRINTCASE(ELEMENT_TOK_UNDERSCORE)
            PRINTCASE(ELEMENT_TOK_DOT)
            PRINTCASE(ELEMENT_TOK_BRACKETL)
            PRINTCASE(ELEMENT_TOK_BRACKETR)
            PRINTCASE(ELEMENT_TOK_COLON)
            PRINTCASE(ELEMENT_TOK_COMMA)
            PRINTCASE(ELEMENT_TOK_BRACEL)
            PRINTCASE(ELEMENT_TOK_BRACER)
            PRINTCASE(ELEMENT_TOK_EQUALS)
        default:
            c = "";
        }

        string += c;

        if (token.tok_pos >= 0 && token.tok_len >= 0)
        {
            //Align the token text using tabs (with tabs = 4 spaces), so that they start at character 24 (4*6)
            auto chunks = c.length() / 4;
            for (auto i = chunks; chunks < 6; chunks++)
                string += "\t";

            auto text = context->text(&token);
            string += text;

            if (nearest_token == &token)
            {
                string += " <--- HERE";
            }
        }

        string += "\n";
    }

    return string;
}

std::string ast_to_string(const element_ast* ast, int depth, const element_ast* ast_to_mark)
{
    std::string string;

    for (int i = 0; i < depth; ++i)
        string += "  ";

    if (ast->type == ELEMENT_AST_NODE_LITERAL)
    {
        string += fmt::format("LITERAL: {}", ast->literal);
    }
    else if (ast->type == ELEMENT_AST_NODE_IDENTIFIER)
    {
        string += fmt::format("IDENTIFIER: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_DECLARATION)
    {
        const auto intrinsic = ast->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
        intrinsic
            ? string += fmt::format("INTRINSIC DECLARATION: {}", ast->identifier)
            : string += fmt::format("DECLARATION: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_NAMESPACE)
    {
        string += fmt::format("NAMESPACE: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_CALL)
    {
        string += fmt::format("CALL: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_PORT)
    {
        string += fmt::format("PORT: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE && ast->parent && ast->parent->type == ELEMENT_AST_NODE_PORT)
    {
        string += "IMPLICIT TYPE";
    }
    else if (ast->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE && ast->parent && ast->parent->type == ELEMENT_AST_NODE_DECLARATION)
    {
        string += "IMPLICIT RETURN";
    }
    else if (ast->type == ELEMENT_AST_NODE_NO_BODY)
    {
        string += "NO BODY";
    }
    else if (ast->type == ELEMENT_AST_NODE_NONE)
    {
        if (ast->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT))
            string += "EMPTY INPUT";
        else if (ast->flags == 0)
            string += "NONE";
        else
            string += "UNKNOWN FLAGS";
    }
    else
    {
        char* c;
        switch (ast->type)
        {
            PRINTCASE(ELEMENT_AST_NODE_ROOT)
            PRINTCASE(ELEMENT_AST_NODE_SCOPE)
            PRINTCASE(ELEMENT_AST_NODE_CONSTRAINT)
            PRINTCASE(ELEMENT_AST_NODE_FUNCTION)
            PRINTCASE(ELEMENT_AST_NODE_STRUCT)
            PRINTCASE(ELEMENT_AST_NODE_EXPRESSION)
            PRINTCASE(ELEMENT_AST_NODE_EXPRLIST)
            PRINTCASE(ELEMENT_AST_NODE_PORTLIST)
            PRINTCASE(ELEMENT_AST_NODE_TYPENAME)
            PRINTCASE(ELEMENT_AST_NODE_LAMBDA)
        default:
            c = "ELEMENT_AST_NODE_<UNKNOWN>";
            break;
        }

        //Offset pointer by length of prefix to cutoff prefix
        string += fmt::format("{}", c + strlen("ELEMENT_AST_NODE_"));
    }

    if (ast_to_mark && ast_to_mark == ast)
        string += " <--- Here";

    string += "\n";

    for (const auto& child : ast->children)
        string += ast_to_string(child.get(), depth + 1, ast_to_mark);

    return string;
}

std::string expression_to_string(const element_expression& expression, std::size_t depth)
{
    std::string string(depth, ' ');

    if (expression.is<element_expression_constant>())
    {
        const auto& constant = expression.as<element_expression_constant>();
        string += "CONSTANT: " + std::to_string(constant->value());
    }

    if (expression.is<element_expression_input>())
    {
        const auto& input = expression.as<element_expression_input>();
        string += "INPUT: scope = " + std::to_string(input->scope()) + "; index = " + std::to_string(input->index());
    }

    if (expression.is<element_expression_serialised_structure>())
    {
        const auto& structure = expression.as<element_expression_serialised_structure>();
        string += "STRUCTURE: ";
    }

    if (expression.is<element_expression_nullary>())
    {
        const auto& nullary = expression.as<element_expression_nullary>();
        string += "NULLARY: ";
        char* c = nullptr;
        switch (nullary->operation())
        {
            //num
            PRINTCASE(element_nullary_op::nan)
            PRINTCASE(element_nullary_op::positive_infinity)
            PRINTCASE(element_nullary_op::negative_infinity)

            //boolean
            PRINTCASE(element_nullary_op::true_value)
            PRINTCASE(element_nullary_op::false_value)
        }
        string += c;
    }

    if (expression.is<element_expression_unary>())
    {
        const auto& unary = expression.as<element_expression_unary>();
        string += "UNARY: ";
        char* c = nullptr;
        switch (unary->operation())
        {
            //num
            PRINTCASE(element_unary_op::sin)
            PRINTCASE(element_unary_op::cos)
            PRINTCASE(element_unary_op::tan)
            PRINTCASE(element_unary_op::asin)
            PRINTCASE(element_unary_op::acos)
            PRINTCASE(element_unary_op::atan)
            PRINTCASE(element_unary_op::ln)
            PRINTCASE(element_unary_op::abs)
            PRINTCASE(element_unary_op::ceil)
            PRINTCASE(element_unary_op::floor)

            //boolean
            PRINTCASE(element_unary_op::not_)
        }
        string += c;
    }

    if (expression.is<element_expression_binary>())
    {
        const auto& binary = expression.as<element_expression_binary>();
        string += "BINARY: ";
        char* c = nullptr;
        switch (binary->operation())
        {
            //num
            PRINTCASE(element_binary_op::add)
            PRINTCASE(element_binary_op::sub)
            PRINTCASE(element_binary_op::mul)
            PRINTCASE(element_binary_op::div)
            PRINTCASE(element_binary_op::rem)
            PRINTCASE(element_binary_op::pow)
            PRINTCASE(element_binary_op::min)
            PRINTCASE(element_binary_op::max)
            PRINTCASE(element_binary_op::log)
            PRINTCASE(element_binary_op::atan2)

            //boolean
            PRINTCASE(element_binary_op::and_)
            PRINTCASE(element_binary_op::or_)

            //comparison
            PRINTCASE(element_binary_op::eq)
            PRINTCASE(element_binary_op::neq)
            PRINTCASE(element_binary_op::lt)
            PRINTCASE(element_binary_op::leq)
            PRINTCASE(element_binary_op::gt)
            PRINTCASE(element_binary_op::geq)
        }
        string += c;
    }

    if (expression.is<element_expression_if>())
    {
        const auto& if_instruction = expression.as<element_expression_if>();
        string += "IF:\n";
        string += std::string(depth + 1, ' ') + "PREDICATE:\n" + expression_to_string(*if_instruction->predicate().get(), depth + 2);
        string += std::string(depth + 1, ' ') + "IF_TRUE:\n" + expression_to_string(*if_instruction->if_true().get(), depth + 2);
        string += std::string(depth + 1, ' ') + "IF_FALSE:\n" + expression_to_string(*if_instruction->if_false().get(), depth + 2);
        return string;
    }

    if (expression.is<element_expression_for>())
    {
        const auto& for_instruction = expression.as<element_expression_for>();
        string += "FOR:" + fmt::format(" [{}]", fmt::ptr(&expression)) + "\n";
        string += std::string(depth + 1, ' ') + "INITIAL:\n" + expression_to_string(*for_instruction->initial().get(), depth + 2);
        string += std::string(depth + 1, ' ') + "CONDITION:\n" + expression_to_string(*for_instruction->condition().get(), depth + 2);
        string += std::string(depth + 1, ' ') + "BODY:\n" + expression_to_string(*for_instruction->body().get(), depth + 2);
        return string;
    }

    if (expression.is<element_expression_select>())
    {
        const auto& select_instruction = expression.as<element_expression_select>();
        string += "SELECT:\n";
        string += std::string(depth + 1, ' ') + "SELECTOR:\n" + expression_to_string(*select_instruction->selector.get(), depth + 2);

        string += std::string(depth + 1, ' ') + "OPTIONS:\n";
        for (const auto& dependent : select_instruction->options)
            string += expression_to_string(*dependent, depth + 2);

        return string;
    }

    if (expression.is<element_expression_indexer>())
    {
        const auto& indexer_instruction = expression.as<element_expression_indexer>();
        string += "INDEXER\n";
        string += expression_to_string(*indexer_instruction->for_expression, depth + 1);
        string += std::string(depth + 1, ' ') + "INDEX: " + std::to_string(indexer_instruction->index) + "\n";

        return string;
    }

    string += "\n";

    for (const auto& dependent : expression.dependents())
        string += expression_to_string(*dependent, depth + 1);
    return string;
}

void element_log_ctx::log(const element_tokeniser_ctx& context, element_result code, const std::string& message, int length, element_log_message* related_message) const
{
    element_log_message msg;
    msg.line = context.line;
    msg.character = context.character;
    msg.stage = ELEMENT_STAGE_TOKENISER;
    msg.filename = context.filename.c_str();
    msg.related_log_message = nullptr;
    msg.line_in_source = nullptr;

    msg.message_code = code;
    msg.length = length;
    msg.related_log_message = related_message;

    std::string new_log_message = message;

    if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens))
        new_log_message += "\n\nTOKENS\n------\n" + tokens_to_string(&context, nullptr);

    msg.message = new_log_message.c_str();
    msg.message_length = static_cast<int>(new_log_message.length());

    log(msg);
}

void element_log_ctx::log(const element_interpreter_ctx& context, element_result code, const std::string& message, const std::string& filename) const
{
    auto msg = element_log_message();
    msg.message = message.c_str();
    msg.message_length = static_cast<int>(message.length());
    msg.message_code = code;
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = ELEMENT_STAGE_EVALUATOR;
    msg.filename = filename.c_str();
    msg.related_log_message = nullptr;
    msg.line_in_source = nullptr;

    log(msg);
}

void element_log_ctx::log(const std::string& message, const element_stage stage) const
{
    auto msg = element_log_message();
    msg.message = message.c_str();
    msg.message_length = static_cast<int>(message.length());
    msg.message_code = ELEMENT_OK;
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = stage;
    msg.related_log_message = nullptr;
    msg.line_in_source = nullptr;

    log(msg);
}

void element_log_ctx::log(const element_parser_ctx& context, element_result code, const std::string& message, const element_ast* nearest_ast) const
{
    assert(context.tokeniser);

    const bool starts_with_prelude = context.tokeniser->filename.rfind("Prelude\\", 0) == 0;
    if (starts_with_prelude && !flag_set(logging_bitmask, log_flags::debug | log_flags::output_prelude))
    {
        return; //early out if prelude logging disabled
    }

    auto msg = element_log_message();
    msg.message_code = code;
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = ELEMENT_STAGE_PARSER;
    msg.filename = context.tokeniser->filename.c_str();
    msg.related_log_message = nullptr;

    std::string new_log_message = message;
    std::string source_line;
    if (nearest_ast && nearest_ast->nearest_token)
    {
        msg.line = nearest_ast->nearest_token->line;
        msg.character = nearest_ast->nearest_token->character;
        msg.length = nearest_ast->nearest_token->tok_len;

        if (msg.line > 0)
            source_line = context.tokeniser->text_on_line(msg.line);
    }

    //Only print ast/token prelude info if it's forced on or if a non-zero code is being logged
    if (code != ELEMENT_OK)
    {
        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens))
            new_log_message += "\n\nTOKENS\n------\n" + tokens_to_string(context.tokeniser, nearest_ast ? nearest_ast->nearest_token : nullptr);

        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast))
            new_log_message += "\n\nAST\n---\n" + ast_to_string(context.root, 0, nearest_ast ? nearest_ast : nullptr);
    }

    msg.message = new_log_message.c_str();
    msg.message_length = static_cast<int>(new_log_message.length());
    msg.line_in_source = source_line.empty() ? nullptr : source_line.c_str();
    log(msg);
}

void element_log_ctx::log(const element_compiler_ctx& context, element_result code, const std::string& message,
                          const element_ast* nearest_ast) const
{
    auto msg = element_log_message();
    msg.message = message.c_str();
    msg.message_length = static_cast<int>(message.length());
    msg.message_code = code;
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = ELEMENT_STAGE_COMPILER;
    msg.filename = "<unknown>"; //todo: get from scope/ast?
    msg.related_log_message = nullptr;
    msg.line_in_source = nullptr;

    std::string new_log_message;
    if (nearest_ast && nearest_ast->nearest_token)
    {
        msg.line = nearest_ast->nearest_token->line;
        msg.character = nearest_ast->nearest_token->character;
        msg.length = nearest_ast->nearest_token->tok_len;

        //todo: print source code/use james debug recreation
        new_log_message = message;
    }
    else
    {
        new_log_message = message;
    }

    if (code != ELEMENT_OK)
    {
        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast))
            new_log_message += "\n\nAST\n---\n" + ast_to_string(get_root_from_ast(nearest_ast), 0, nearest_ast);
    }

    msg.message = new_log_message.c_str();

    log(msg);
}

void element_log_ctx::log(const element_log_message& log) const
{
    if (callback == nullptr)
        return;

    callback(&log, user_data);
}

void element_log_ctx::log(const element::log_message& log) const
{
    if (callback == nullptr)
        return;

    callback(&log.get_log_message(), user_data);
}