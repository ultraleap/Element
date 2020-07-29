#pragma once

//STD
#include <memory>

//SELF
#include "ast/fwd.hpp"
#include "fwd.hpp"

namespace element
{
    template <typename T>
    void assign_source_information(const element_interpreter_ctx* context, T& t, const element_ast* ast)
    {
        const auto& file_info = context->src_context->file_info.at(ast->nearest_token->file_name);
        const std::string* filename = file_info.file_name.get();
        const std::string* line_in_source = file_info.source_lines[ast->nearest_token->line - 1].get();
        t->source_info = source_information(
            ast->nearest_token->line,
            ast->nearest_token->character,
            ast->nearest_token->character + ast->nearest_token->tok_len,
            line_in_source,
            filename->data()
        );
    }

    std::unique_ptr<expression> build_expression(const element_interpreter_ctx* context, const element_ast* ast, expression_chain* chain, element_result& output_result);
    std::unique_ptr<expression_chain> build_expression_chain(const element_interpreter_ctx* context, const element_ast* const ast, const declaration* declarer, element_result& output_result);
	std::unique_ptr<scope> build_root_scope(const element_interpreter_ctx* context, const element_ast* ast, element_result& output_result);
}

