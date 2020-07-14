#include "object_model.hpp"

//STD
#include <iostream>

//SELF
#include "expressions.hpp"
#include "configuration.hpp"
#include "intrinsics.hpp"

namespace element
{
    void build_scope(const element_interpreter_ctx* context, element_ast* ast, const declaration& declaration);

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

    std::unique_ptr<type_annotation> build_type_annotation(const element_interpreter_ctx* context, const element_ast* ast)
    {
        //todo: we need to decide how to handle any and num as types when they're not included (as the prelude is missing)
        if (ast->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE)
            return nullptr;

        if (ast->type == ELEMENT_AST_NODE_TYPENAME)
        {
            auto* const ident = ast->children[ast_idx::port::type].get();
            if (ident->type != ELEMENT_AST_NODE_IDENTIFIER)
                throw;

            return std::make_unique<type_annotation>(identifier(ident->identifier));
        }

        throw;
    }

    void build_output(const element_interpreter_ctx* context, element_ast* ast, declaration& declaration)
    {
        auto* const output = ast->children[ast_idx::declaration::outputs].get();
        auto type_annotation = build_type_annotation(context, output);
        declaration.output.emplace(port{ identifier::return_identifier, std::move(type_annotation) });
    }

    void build_inputs(const element_interpreter_ctx* context, element_ast* ast, declaration& declaration)
    {
        auto* const inputs = ast->children[ast_idx::declaration::inputs].get();

        for (auto& input : inputs->children)
        {
            if (input->type != ELEMENT_AST_NODE_PORT)
                throw;

            auto ident = identifier(input->identifier);

            const auto has_type_annotation = input->children.size() > ast_idx::port::type;
            if (!has_type_annotation)
            {
                declaration.inputs.emplace_back(ident, nullptr);
                continue;
            }

            auto* const output = input->children[ast_idx::port::type].get();
            auto type_annotation = build_type_annotation(context, output);
            declaration.inputs.emplace_back(ident, std::move(type_annotation));
        }
    }

    std::shared_ptr<declaration> build_struct_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto struct_decl = std::make_unique<struct_declaration>(identifier(decl->identifier), parent_scope, intrinsic);

        //fields
        build_inputs(context, decl, *struct_decl);
        build_output(context, decl, *struct_decl);

        if (ast->children.size() > ast_idx::function::body)
        {
            auto* body = ast->children[ast_idx::function::body].get();
            if (body->type == ELEMENT_AST_NODE_SCOPE)
                build_scope(context, body, *struct_decl);
        }

        return std::move(struct_decl);
    }

    std::shared_ptr<declaration> build_constraint_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto constraint_decl = std::make_unique<constraint_declaration>(identifier(decl->identifier), intrinsic);

        //ports
        build_inputs(context, decl, *constraint_decl);
        build_output(context, decl, *constraint_decl);

        return std::move(constraint_decl);
    }

    std::shared_ptr<declaration> build_function_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto function_decl = std::make_shared<function_declaration>(identifier(decl->identifier), parent_scope, intrinsic);
        assign_source_information(context, function_decl, decl);

        build_inputs(context, decl, *function_decl);
        build_output(context, decl, *function_decl);

        auto* const body = ast->children[ast_idx::function::body].get();

        if (body->type == ELEMENT_AST_NODE_SCOPE)
        {
            assert(!intrinsic);
            build_scope(context, body, *function_decl);
            function_decl->body = function_decl->our_scope->find(identifier::return_identifier, false);
            if (!function_decl->body)
            {
                //todo: commented out as parsing files that contain lambdas etc cause issues even if not used
                //assert(!"scope-bodied function is missing a return");
            }
        }
        else if (body->type == ELEMENT_AST_NODE_CALL || body->type == ELEMENT_AST_NODE_LITERAL)
        {
            assert(!intrinsic);
            auto chain = std::make_unique<expression_chain>(function_decl.get());
            assign_source_information(context, chain, body);
            build_expression(context, body, chain.get());
            function_decl->body = std::move(chain);
        }
        else if (intrinsic && body->type == ELEMENT_AST_NODE_NO_BODY)
        {
            assert(intrinsic);
            //TODO: If get_intrinsic returns null return error ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED
            function_decl->body = intrinsic::get_intrinsic(*function_decl);
        }
        else
        {
            function_decl = nullptr;
        }

        return function_decl;
    }

    void build_call_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain)
    {
        if (chain->expressions.empty())
        {
            assert(!"found a call expression_chain at the start of a chain");
            return;
        }

        if (ast->children.empty())
        {
            assert(!"found a call expression_chain with no children (arguments)");
            return;
        }

        auto call_expr = std::make_unique<call_expression>(chain);
        assign_source_information(context, call_expr, ast);

        //every child of the current AST node (EXPRLIST) is the start of another expression_chain, comma separated
        for (const auto& child : ast->children)
        {
            auto argument = std::make_unique<expression_chain>(chain->declarer);
            assign_source_information(context, argument, child.get());

            build_expression(context, child.get(), argument.get());
            call_expr->arguments.push_back(std::move(argument));
        }

        chain->expressions.push_back(std::move(call_expr));
    }

    void build_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain)
    {
        assert(chain);
        assert(chain->declarer);
        const auto is_literal = ast->type == ELEMENT_AST_NODE_LITERAL;
        const auto is_lambda = ast->type == ELEMENT_AST_NODE_LAMBDA;
        const auto is_identifier = ast->type == ELEMENT_AST_NODE_CALL;
        const auto is_call = ast->type == ELEMENT_AST_NODE_EXPRLIST;

        if (is_lambda)
        {
            chain->expressions.push_back(std::make_unique<lambda_expression>(chain));
            assign_source_information(context, chain->expressions.back(), ast);
            return;
        }

        if (is_call)
        {
            build_call_expression(context, ast, chain);
        }

        if (is_literal)
        {
            if (!chain->expressions.empty())
            {
                assert(!"found literal in the middle of a chain");
            }

            chain->expressions.push_back(std::make_unique<literal_expression>(ast->literal, chain));
            assign_source_information(context, chain->expressions.back(), ast);
        }
        else if (is_identifier)
        {
            if (chain->expressions.empty())
                chain->expressions.push_back(std::make_unique<identifier_expression>(ast->identifier, chain));
            else
                chain->expressions.push_back(std::make_unique<indexing_expression>(ast->identifier, chain));

            assign_source_information(context, chain->expressions.back(), ast);
        }

        //start of an expression chain, build the rest of it
        if (chain->expressions.size() == 1)
        {
            //every child of the first AST node is part of the chain
            for (const auto& child : ast->children)
            {
                build_expression(context, child.get(), chain);
            }
        }
    }

    std::shared_ptr<declaration> build_namespace_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope)
    {
        auto namespace_decl = std::make_unique<namespace_declaration>(identifier(ast->identifier), parent_scope);
        assign_source_information(context, namespace_decl, ast);

        if (ast->children.size() > ast_idx::ns::body)
        {
            auto* body = ast->children[ast_idx::ns::body].get();
            if (body->type == ELEMENT_AST_NODE_SCOPE)
                build_scope(context, body, *namespace_decl);
        }

        return std::move(namespace_decl);
    }

    std::shared_ptr<declaration> build_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope)
    {
        std::shared_ptr<declaration> result;
        if (ast->type == ELEMENT_AST_NODE_STRUCT)
            result = build_struct_declaration(context, ast, parent_scope);

        if (ast->type == ELEMENT_AST_NODE_CONSTRAINT)
            result = build_constraint_declaration(context, ast, parent_scope);

        if (ast->type == ELEMENT_AST_NODE_FUNCTION)
            result = build_function_declaration(context, ast, parent_scope);

        if (ast->type == ELEMENT_AST_NODE_NAMESPACE)
            result = build_namespace_declaration(context, ast, parent_scope);

        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_typeof_information))
            std::cout << result->typeof_info() + "\n";

        return result;
    }

    void build_scope(const element_interpreter_ctx* context, element_ast* ast, const declaration& declaration)
    {
        for (const auto& child : ast->children)
        {
            auto child_decl = build_declaration(context, child.get(), declaration.our_scope.get());
            if (child_decl)
                declaration.our_scope->add_declaration(std::move(child_decl));
        }
    }

    std::unique_ptr<scope> build_root_scope(const element_interpreter_ctx* context, const element_ast* const ast)
    {
        if (ast->type != ELEMENT_AST_NODE_ROOT)
            return nullptr;

        auto root = std::make_unique<scope>(nullptr, nullptr);
        assign_source_information(context, root, ast);

        for (const auto& child : ast->children)
        {
            auto decl = build_declaration(context, child.get(), root.get());
            if (decl)
                root->add_declaration(std::move(decl));
        }

        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_object_model_as_code)) {
            std::cout << "\n<CODE>\n";
            std::cout << root->to_code();
            std::cout << "\n</CODE>\n\n";
        }

        return root;
    }
}