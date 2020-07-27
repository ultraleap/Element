#include "object_model.hpp"

//STD
#include <iostream>

//SELF
#include "expressions.hpp"
#include "configuration.hpp"
#include "intrinsics.hpp"

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

    void build_scope(const element_interpreter_ctx* context, const element_ast* ast, const declaration& declaration, element_result& output_result);
    void build_scope(const element_interpreter_ctx* context, const element_ast* ast, scope* our_scope, element_result& output_result);
    std::shared_ptr<declaration> build_struct_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope, element_result& output_result);
    std::shared_ptr<declaration> build_constraint_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope, element_result& output_result);
    std::shared_ptr<declaration> build_function_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope, element_result& output_result);
    std::shared_ptr<declaration> build_namespace_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope, element_result& output_result);
    std::shared_ptr<declaration> build_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope, element_result& output_result);
    void build_expression(const element_interpreter_ctx* context, const element_ast* ast, expression_chain* chain, element_result& output_result);

    std::unique_ptr<type_annotation> build_type_annotation(const element_interpreter_ctx* context, const element_ast* ast, element_result& output_result)
    {
        //todo: instead of nullptr, use an object to represent nothing? can't use Any, as user might not have it in source
        if (ast->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE)
            return nullptr;

        if (ast->type == ELEMENT_AST_NODE_TYPENAME)
        {
            auto* const ident = ast->children[ast_idx::port::type].get();
            if (ident->type != ELEMENT_AST_NODE_IDENTIFIER)
            {
                output_result = log_error(context, context->src_context.get(), ident, log_error_message_code::invalid_type_annotation, ast->parent->parent->identifier);
                return nullptr;
            }

            auto element = std::make_unique<type_annotation>(identifier(ident->identifier));
            assign_source_information(context, element, ast);
            return element;
        }

        output_result = log_error(context, context->src_context.get(), ast, log_error_message_code::invalid_type_annotation, ast->parent->parent->identifier);
        return nullptr;
    }

    void build_output(const element_interpreter_ctx* context, element_ast* ast, declaration& declaration, element_result& output_result)
    {
        auto* const output = ast->children[ast_idx::declaration::outputs].get();
        auto type_annotation = build_type_annotation(context, output, output_result);
        declaration.output.emplace(port{ identifier::return_identifier, std::move(type_annotation) });
    }

    void build_inputs(const element_interpreter_ctx* context, element_ast* ast, declaration& declaration, element_result& output_result)
    {
        auto* const inputs = ast->children[ast_idx::declaration::inputs].get();

        for (auto& input : inputs->children)
        {
            if (input->type != ELEMENT_AST_NODE_PORT)
            {
                output_result = log_error(context, context->src_context.get(), input.get(), log_error_message_code::invalid_grammar_in_portlist, declaration.name.value);
                return;
            }

            auto ident = identifier(input->identifier);

            //todo: is either not there, or is UNSPECIFIED_TYPE, need to clean up AST to only do one or the other. seems like inputs are always missing the child, outputs have unspecified type
            const auto has_type_annotation = input->children.size() > ast_idx::port::type;
            if (!has_type_annotation)
            {
                //todo: instead of nullptr, use an object to represent nothing? can't use Any, as user might not have it in source
                declaration.inputs.emplace_back(ident, nullptr);
                continue;
            }

            auto* const output = input->children[ast_idx::port::type].get();
            auto type_annotation = build_type_annotation(context, output, output_result);
            declaration.inputs.emplace_back(ident, std::move(type_annotation));
        }
    }

    std::shared_ptr<declaration> build_struct_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        const auto is_intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto struct_decl = std::make_unique<struct_declaration>(identifier(decl->identifier), parent_scope, is_intrinsic);

        //fields
        build_inputs(context, decl, *struct_decl, output_result);
        build_output(context, decl, *struct_decl, output_result);

        if (is_intrinsic)
        {
            //todo: handle the intrinsic not existing once we have them all (don't break using the prelude for everything else)
            intrinsic::register_intrinsic<struct_declaration>(context, ast, *struct_decl);
        }

        if (ast->children.size() > ast_idx::function::body)
        {
            auto* body = ast->children[ast_idx::function::body].get();
            if (body->type == ELEMENT_AST_NODE_SCOPE)
                build_scope(context, body, *struct_decl, output_result);
        }

        return std::move(struct_decl);
    }

    std::shared_ptr<declaration> build_constraint_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto constraint_decl = std::make_unique<constraint_declaration>(identifier(decl->identifier), intrinsic);

        //ports
        build_inputs(context, decl, *constraint_decl, output_result);
        build_output(context, decl, *constraint_decl, output_result);

        if (intrinsic)
        {
            intrinsic::register_intrinsic<struct_declaration>(context, ast, *constraint_decl);
        }

        return std::move(constraint_decl);
    }

    std::shared_ptr<declaration> build_function_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto function_decl = std::make_shared<function_declaration>(identifier(decl->identifier), parent_scope, intrinsic);
        assign_source_information(context, function_decl, decl);

        build_inputs(context, decl, *function_decl, output_result);
        build_output(context, decl, *function_decl, output_result);

        auto* const body = ast->children[ast_idx::function::body].get();

        if (body->type == ELEMENT_AST_NODE_SCOPE)
        {
            assert(!intrinsic);
            build_scope(context, body, *function_decl, output_result);
            function_decl->body = function_decl->our_scope->find(identifier::return_identifier, false);
            if (!function_decl->body)
            {
                //todo: log error
                output_result = ELEMENT_ERROR_MISSING_FUNCTION_BODY;
                return nullptr;
            }
        }
        else if (body->type == ELEMENT_AST_NODE_CALL || body->type == ELEMENT_AST_NODE_LITERAL || body->type == ELEMENT_AST_NODE_LAMBDA)
        {
            assert(!intrinsic);
            auto chain = std::make_unique<expression_chain>(function_decl.get());
            assign_source_information(context, chain, body);
            build_expression(context, body, chain.get(), output_result);
            function_decl->body = std::move(chain);
        }
        else if (intrinsic && body->type == ELEMENT_AST_NODE_NO_BODY)
        {
            assert(intrinsic);
            if (intrinsic::register_intrinsic<function_declaration>(context, ast, *function_decl))
                function_decl->body = intrinsic::get_intrinsic(context, *function_decl);
        }
        else
        {
            //todo: log error
            function_decl = nullptr;
            output_result = ELEMENT_ERROR_UNKNOWN;
        }

        return function_decl;
    }

    void build_call_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, element_result& output_result)
    {
        if (chain->expressions.empty())
        {
            assert(!"found a call expression_chain at the start of a chain");
            output_result = ELEMENT_ERROR_UNKNOWN;
            return;
        }

        if (ast->children.empty())
        {
            assert(!"found a call expression_chain with no children (arguments)");
            output_result = ELEMENT_ERROR_UNKNOWN;
            return;
        }

        auto call_expr = std::make_unique<call_expression>(chain);
        assign_source_information(context, call_expr, ast);

        //every child of the current AST node (EXPRLIST) is the start of another expression_chain, comma separated
        for (const auto& child : ast->children)
        {
            auto argument = std::make_unique<expression_chain>(chain->declarer);
            assign_source_information(context, argument, child.get());

            build_expression(context, child.get(), argument.get(), output_result);
            call_expr->arguments.push_back(std::move(argument));
        }

        chain->expressions.push_back(std::move(call_expr));
    }

    void build_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, element_result& output_result)
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
            build_call_expression(context, ast, chain, output_result);
        }

        if (is_literal)
        {
            if (!chain->expressions.empty())
            {
                assert(!"found literal in the middle of a chain");
                output_result = ELEMENT_ERROR_UNKNOWN;
                return;
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
                build_expression(context, child.get(), chain, output_result);
            }
        }
    }

    std::shared_ptr<declaration> build_namespace_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto namespace_decl = std::make_unique<namespace_declaration>(identifier(ast->identifier), parent_scope);
        assign_source_information(context, namespace_decl, ast);

        if (ast->children.size() > ast_idx::ns::body)
        {
            auto* body = ast->children[ast_idx::ns::body].get();
            if (body->type == ELEMENT_AST_NODE_SCOPE)
                build_scope(context, body, *namespace_decl, output_result);
        }

        return std::move(namespace_decl);
    }

    std::shared_ptr<declaration> build_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        std::shared_ptr<declaration> result;
        if (ast->type == ELEMENT_AST_NODE_STRUCT)
            result = build_struct_declaration(context, ast, parent_scope, output_result);

        if (ast->type == ELEMENT_AST_NODE_CONSTRAINT)
            result = build_constraint_declaration(context, ast, parent_scope, output_result);

        if (ast->type == ELEMENT_AST_NODE_FUNCTION)
            result = build_function_declaration(context, ast, parent_scope, output_result);

        if (ast->type == ELEMENT_AST_NODE_NAMESPACE)
            result = build_namespace_declaration(context, ast, parent_scope, output_result);

        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_typeof_information))
            std::cout << result->typeof_info() + "\n";

        return result;
    }

    void build_scope(const element_interpreter_ctx* context, const element_ast* ast, scope* our_scope, element_result& output_result)
    {
        for (const auto& child : ast->children)
        {
            element_result result = ELEMENT_OK;
            auto decl = build_declaration(context, child.get(), our_scope, result);

            if (!decl && result == ELEMENT_OK)
                result = ELEMENT_ERROR_UNKNOWN;

            if (result != ELEMENT_OK)
            {
                std::string identifier = child->children.empty() ? "<unknown>" : child->children[0]->identifier;
                log_error(context, context->src_context.get(), child.get(), log_error_message_code::failed_to_build_declaration, std::move(identifier));

                if (output_result == ELEMENT_OK)
                    output_result = result;

                continue;
            }

            //only reaches here if the declaration is nullptr, and no error was reported. should never happen.
            if (!decl)
            {
                result = log_error(context, context->src_context.get(), child.get(), log_error_message_code::internal_compiler_error);
                if (output_result == ELEMENT_OK)
                    output_result = result;
                continue;
            }

            assert(decl);
            our_scope->add_declaration(std::move(decl));
        }
    }

    void build_scope(const element_interpreter_ctx* context, const element_ast* ast, const declaration& declaration, element_result& output_result)
    {
        build_scope(context, ast, declaration.our_scope.get(), output_result);
    }

    std::unique_ptr<scope> build_root_scope(const element_interpreter_ctx* context, const element_ast* const ast, element_result& output_result)
    {
        if (ast->type != ELEMENT_AST_NODE_ROOT)
        {
            output_result = log_error(context, context->src_context.get(), ast, log_error_message_code::failed_to_build_declaration, ast->nearest_token->file_name);
            return nullptr;
        }

        auto root = std::make_unique<scope>(nullptr, nullptr);
        assign_source_information(context, root, ast);

        build_scope(context, ast, root.get(), output_result);

        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_object_model_as_code)) {
            std::cout << "\n<CODE>\n";
            std::cout << root->to_code();
            std::cout << "\n</CODE>\n\n";
        }

        return root;
    }
}