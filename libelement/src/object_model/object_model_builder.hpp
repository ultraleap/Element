#pragma once

//STD
#include <memory>

//SELF
#include "ast/fwd.hpp"
#include "fwd.hpp"
#include "source_information.hpp"

namespace element
{
    //todo: all of this is templated due to GCC having issues with it, move somewhere else where we can have complete types
    //I imagine MSVC does its checks when the template is instantiated, where as GCC does it when it's parsed
    template <typename Context, typename Obj, typename AST>
    void assign_source_information(const Context* context, Obj& t, const AST* ast)
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

    typedef std::vector<std::pair<identifier, const element_ast*>> deferred_expressions;

    std::unique_ptr<declaration> build_lambda_declaration(const element_interpreter_ctx* context, identifier& identifier, const element_ast* expression, const scope* parent_scope, element_result& output_result);
    std::unique_ptr<expression> build_expression(const element_interpreter_ctx* context, const element_ast* ast, expression_chain* chain, deferred_expressions& deferred_expressions, element_result& output_result);
    std::unique_ptr<expression_chain> build_expression_chain(const element_interpreter_ctx* context, const element_ast* const ast, const declaration* declarer, deferred_expressions& deferred_expressions, element_result& output_result);
	std::unique_ptr<scope> build_root_scope(const element_interpreter_ctx* context, const element_ast* ast, element_result& output_result);
}

