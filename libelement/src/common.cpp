#pragma once

#include <iostream>

#include "common_internal.hpp"
#include "token_internal.hpp"
#include "ast/ast_internal.hpp"
#include "etree/compiler.hpp"

#include <fmt/format.h>

#include "configuration.hpp"

#define PRINTCASE(a) case a: c = #a; break;
std::string tokens_to_string(const element_tokeniser_ctx* tokeniser, const element_token* nearest_token = nullptr)
{
    std::string string;

	for(const auto& token : tokeniser->tokens)
	{
        std::string c;
        switch (token.type)
        {
            PRINTCASE(ELEMENT_TOK_NONE);
            PRINTCASE(ELEMENT_TOK_NUMBER);
            PRINTCASE(ELEMENT_TOK_IDENTIFIER);
            PRINTCASE(ELEMENT_TOK_UNDERSCORE);
            PRINTCASE(ELEMENT_TOK_DOT);
            PRINTCASE(ELEMENT_TOK_BRACKETL);
            PRINTCASE(ELEMENT_TOK_BRACKETR);
            PRINTCASE(ELEMENT_TOK_SEMICOLON);
            PRINTCASE(ELEMENT_TOK_COLON);
            PRINTCASE(ELEMENT_TOK_COMMA);
            PRINTCASE(ELEMENT_TOK_BRACEL);
            PRINTCASE(ELEMENT_TOK_BRACER);
            PRINTCASE(ELEMENT_TOK_EQUALS);
			default: c = "";
        }
		
        string += c;

		if(token.tok_pos >= 0 && token.tok_len >= 0)
		{
            //Align the token text using tabs (with tabs = 4 spaces), so that they start at character 24 (4*6)
            auto chunks = c.length() / 4;
            for(auto i = chunks; chunks < 6; chunks++)
                string += "\t";

            auto text = tokeniser->text(&token);
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

    if (ast->type == ELEMENT_AST_NODE_LITERAL) {
        string += fmt::format("LITERAL: {}", ast->literal);
    }
    else if (ast->type == ELEMENT_AST_NODE_IDENTIFIER) {
        string += fmt::format("IDENTIFIER: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_DECLARATION) {
        const auto intrinsic = ast->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
        intrinsic
            ? string += fmt::format("INTRINSIC DECLARATION: {}", ast->identifier)
            : string += fmt::format("DECLARATION: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_NAMESPACE) {
        string += fmt::format("NAMESPACE: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_CALL) {
        string += fmt::format("CALL: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_PORT) {
        string += fmt::format("PORT: {}", ast->identifier);
    }
    else if (ast->type == ELEMENT_AST_NODE_NONE) {
        if (ast->has_flag(ELEMENT_AST_FLAG_DECL_EMPTY_INPUT))
            string += "EMPTY INPUT";
        else if (ast->has_flag(ELEMENT_AST_FLAG_DECL_IMPLICIT_RETURN))
            string += "IMPLICIT RETURN";
        else if (ast->flags == 0)
            string += "NONE";
        else
            string += "UNKNOWN FLAGS";
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

        //Offset pointer by length of prefix to cutoff prefix
        string += fmt::format("{}", c + strlen("ELEMENT_AST_NODE_"));
    }

    if (ast_to_mark && ast_to_mark == ast)
        string += " <--- Here";

    string += "\n";

    if (has_value(logging_bitmask, log_flags::output_ast)) {
        for (const auto& child : ast->children)
            string += ast_to_string(child.get(), depth + 1, ast_to_mark);
    }

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

    msg.message_code = code;
    msg.length = length;
    msg.related_log_message = related_message;

    std::string new_log_message = message;
    new_log_message += "\n\n" + tokens_to_string(&context);
    msg.message = new_log_message.c_str();

    log(msg);
}

void element_log_ctx::log(const element_interpreter_ctx& context, element_result code, const std::string& message, const std::string& filename) const
{
    auto msg = element_log_message();
    msg.message = message.c_str();
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = ELEMENT_STAGE_PARSER;
    msg.filename = filename.c_str();
    msg.related_log_message = nullptr;
	
    log(msg);
}

void element_log_ctx::log(const element_parser_ctx& context, element_result code, const std::string& message, const element_ast* nearest_ast) const
{
    assert(context.tokeniser);

    auto msg = element_log_message();
    msg.message = message.c_str();
    msg.message_code = code;
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = ELEMENT_STAGE_PARSER;
    msg.filename = context.tokeniser->filename.c_str();
    msg.related_log_message = nullptr;

    std::string new_log_message;
    if (nearest_ast && nearest_ast->nearest_token) {
        msg.line = nearest_ast->nearest_token->line;
        msg.character = nearest_ast->nearest_token->character;
        msg.length = nearest_ast->nearest_token->tok_len;

        if (msg.line > 0) {
            std::string source_line = context.tokeniser->text_on_line(msg.line) + "\n";

            //todo: doesn't handle UTF8 I'm guessing
            if (msg.character >= 0) {
                for (int i = 0; i < msg.character - 1; ++i)
                    source_line += " ";

                source_line += "^";

                for (int i = 0; i < msg.length - 1; ++i)
                    source_line += "^";
            }

            new_log_message = "\n\n" + source_line + " " + message;
        }
    } else {
        new_log_message = message;
    }

    //Only print ast/token prelude info if it's forced on or if a non-zero code is being logged
    const bool starts_with_prelude = context.tokeniser->filename.rfind("Prelude\\", 0) == 0;
    if (!starts_with_prelude || code != ELEMENT_OK || has_value(logging_bitmask, log_flags::output_prelude))
    {
        if (has_value(logging_bitmask, log_flags::output_ast))
			new_log_message += "\n\n" + ast_to_string(context.root, 0, nearest_ast);
    	
        if (has_value(logging_bitmask, log_flags::output_tokens))
			new_log_message += "\n\n" + tokens_to_string(context.tokeniser, nearest_ast ? nearest_ast->nearest_token : nullptr);
    } else {
        new_log_message += "\nskipped printing prelude debug info";
    }
	
    msg.message = new_log_message.c_str();
	
    log(msg);
}

void element_log_ctx::log(const element_compiler_ctx& context, element_result code, const std::string& message,
    const element_ast* nearest_ast) const
{
    auto msg = element_log_message();
    msg.message = message.c_str();
    msg.message_code = code;
    msg.line = -1;
    msg.character = -1;
    msg.length = -1;
    msg.stage = ELEMENT_STAGE_PARSER;
    msg.filename = "unknown"; //todo: get from scope/ast?
    msg.related_log_message = nullptr;

    std::string new_log_message;
    if (nearest_ast && nearest_ast->nearest_token) {
        msg.line = nearest_ast->nearest_token->line;
        msg.character = nearest_ast->nearest_token->character;
        msg.length = nearest_ast->nearest_token->tok_len;

        //todo: print source code/use james debug recreation
        new_log_message = message;
    }
    else {
        new_log_message = message;
    }

    if (nearest_ast) {
        new_log_message += "\n\n" + ast_to_string(get_root_from_ast(nearest_ast), 0, nearest_ast);
    }

    msg.message = new_log_message.c_str();

    log(msg);
}

void element_log_ctx::log(const element_log_message& log) const
{
    if (callback == nullptr)
        return;

    callback(&log);
}
