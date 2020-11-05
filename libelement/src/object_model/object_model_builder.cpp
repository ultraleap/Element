#include "object_model_builder.hpp"

//STD
#include <iostream>
#include <vector>

//SELF
#include "configuration.hpp"
#include "declarations/struct_declaration.hpp"
#include "declarations/function_declaration.hpp"
#include "declarations/namespace_declaration.hpp"
#include "declarations/constraint_declaration.hpp"
#include "expressions/expression.hpp"
#include "expressions/expression_chain.hpp"
#include "expressions/literal_expression.hpp"
#include "expressions/identifier_expression.hpp"
#include "expressions/indexing_expression.hpp"
#include "expressions/call_expression.hpp"
#include "intrinsics/intrinsic.hpp"
#include "log_errors.hpp"
#include "ast/ast_internal.hpp"
#include "expressions/anonymous_block_expression.hpp"

namespace element
{
    //declarations
    void build_scope(const element_interpreter_ctx* context, const element_ast* ast, const declaration& declaration, element_result& output_result);
    void build_scope(const element_interpreter_ctx* context, const element_ast* ast, scope* our_scope, element_result& output_result);

    //definitions
    function_declaration::kind get_function_kind(const element_ast* const body, const bool intrinsic)
    {
        //wrapped in a lambda only to indicate that this initialisation behaviour
        //is grouped and dependent on the items that precede it
        if (intrinsic)
            return function_declaration::kind::intrinsic;

        return body->type == ELEMENT_AST_NODE_SCOPE
                   ? function_declaration::kind::scope_bodied
                   : function_declaration::kind::expression_bodied;
    };

    void flatten_ast(element_ast* root, std::vector<const element_ast*>& vec)
    {
        for (const auto& child : root->children)
        {
            vec.push_back(child.get());
            flatten_ast(child.get(), vec);
        }
    }

    std::unique_ptr<type_annotation> build_type_annotation(const element_interpreter_ctx* context, const element_ast* ast, element_result& output_result)
    {
        //todo: instead of nullptr, use an object to represent nothing? can't use Any, as user might not have it in source
        if (ast->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE)
            return nullptr;

        if (ast->type == ELEMENT_AST_NODE_TYPENAME)
        {
            auto* const ident = ast->children[ast_idx::port::type].get();

            //todo: make expression chain, it's not an identifier

            std::vector<const element_ast*> flattened_ast;
            flatten_ast(ident, flattened_ast);

            std::string type_annotation_string = ident->identifier;
            for (const auto* child : flattened_ast)
                type_annotation_string += "." + child->identifier;

            auto element = std::make_unique<type_annotation>(identifier(type_annotation_string));
            assign_source_information(context, element, ast);
            return element;
        }

        return nullptr;
    }

    void build_output(const element_interpreter_ctx* context, element_ast* output, declaration& declaration, element_result& output_result)
    {
        auto type_annotation = build_type_annotation(context, output, output_result);
        declaration.output.emplace(port{ &declaration, identifier::return_identifier, std::move(type_annotation), nullptr });
    }

    void build_inputs(const element_interpreter_ctx* context, element_ast* inputs, declaration& declaration, element_result& output_result)
    {
        for (auto& input : inputs->children)
        {
            if (input->type != ELEMENT_AST_NODE_PORT)
            {
                output_result = log_error(context, context->src_context.get(), input.get(), log_error_message_code::invalid_grammar_in_portlist, declaration.name.value);
                return;
            }

            auto ident = identifier(input->identifier);

            auto* const type = input->children[ast_idx::port::type].get();
            auto* const default_value = input->children[ast_idx::port::default_value].get();

            std::unique_ptr<expression_chain> chain = nullptr;
            if (default_value->type != ELEMENT_AST_NODE_UNSPECIFIED_DEFAULT)
            {
                deferred_expressions deferred_expressions;
                chain = build_expression_chain(context, default_value, &declaration, deferred_expressions, output_result);
            }

            auto type_annotation = build_type_annotation(context, type, output_result);
            declaration.inputs.emplace_back(&declaration, ident, std::move(type_annotation), std::move(chain));
        }
    }

    void build_inputs_output(const element_interpreter_ctx* context, const element_ast* const ast, declaration& declaration, element_result& output_result, int type)
    {
        element_ast* inputs = nullptr;
        element_ast* output = nullptr;

        if (type == ELEMENT_AST_NODE_LAMBDA)
        {
            inputs = ast->children[ast_idx::lambda::inputs].get();
            output = ast->children[ast_idx::lambda::output].get();
        }
        else
        {
            inputs = ast->children[ast_idx::declaration::inputs].get();
            output = ast->children[ast_idx::declaration::outputs].get();
        }

        build_inputs(context, inputs, declaration, output_result);
        build_output(context, output, declaration, output_result);
    }

    std::unique_ptr<declaration> build_struct_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        const auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto struct_kind = intrinsic
                               ? struct_declaration::kind::intrinsic
                               : struct_declaration::kind::custom;

        auto struct_decl = std::make_unique<struct_declaration>(identifier(decl->identifier), parent_scope, struct_kind);
        build_inputs_output(context, decl, *struct_decl, output_result, ELEMENT_AST_NODE_STRUCT);

        if (intrinsic)
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

        for (const auto& input : struct_decl->inputs)
        {
            if (input.get_name().empty())
                output_result = log_error(context, context->src_context.get(), decl, log_error_message_code::struct_portlist_cannot_contain_discards, struct_decl->name.value);
        }

        if (output_result != ELEMENT_OK)
            return nullptr;

        return std::move(struct_decl);
    }

    std::unique_ptr<declaration> build_constraint_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        const auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto constraint_kind = intrinsic
                                   ? constraint_declaration::kind::intrinsic
                                   : constraint_declaration::kind::custom;

        auto constraint_decl = std::make_unique<constraint_declaration>(identifier(decl->identifier), parent_scope, constraint_kind);

        build_inputs_output(context, decl, *constraint_decl, output_result, ELEMENT_AST_NODE_CONSTRAINT);

        if (intrinsic)
        {
            intrinsic::register_intrinsic<constraint_declaration>(context, ast, *constraint_decl);
        }

        return std::move(constraint_decl);
    }

    std::unique_ptr<declaration> build_lambda_declaration(const element_interpreter_ctx* context, identifier& identifier, const element_ast* const expression, const scope* const parent_scope, element_result& output_result)
    {
        auto* const lambda_body = expression->children[ast_idx::lambda::body].get();

        //TODO: This needs to be a new type e.g. lambda_declaration, at the very least for to_code to function correctly (might require some string magic to resugar/sugarify/???) and be able to distinguish between lambda vs not
        auto lambda_function_decl = std::make_unique<function_declaration>(identifier, parent_scope, get_function_kind(lambda_body, false));
        assign_source_information(context, lambda_function_decl, expression);

        build_inputs_output(context, expression, *lambda_function_decl, output_result, ELEMENT_AST_NODE_LAMBDA);

        if (lambda_body->type == ELEMENT_AST_NODE_SCOPE)
        {
            build_scope(context, lambda_body, *lambda_function_decl, output_result);

            const auto* return_func = lambda_function_decl->our_scope->find(identifier::return_identifier, false);
            if (!return_func)
            {
                output_result = log_error(context, context->src_context.get(), expression, log_error_message_code::function_missing_return, lambda_function_decl->name.value);
                return nullptr;
            }

            lambda_function_decl->body = return_func;
        }
        else
        {
            deferred_expressions deferred_expressions;
            auto chain = build_expression_chain(context, lambda_body, lambda_function_decl.get(), deferred_expressions, output_result);

            if (deferred_expressions.empty())
            {
                assert(!chain->expressions.empty());
                assert(chain->expressions[0]);
                lambda_function_decl->body = std::move(chain);
            }
            else
            {
                for (auto& [nested_identifier, nested_expression] : deferred_expressions)
                {

                    auto lambda = build_lambda_declaration(context, nested_identifier, nested_expression, lambda_function_decl->our_scope.get(), output_result);
                    const auto is_added = lambda_function_decl->our_scope->add_declaration(std::move(lambda));
                    if (!is_added)
                    {
                        //todo: error
                    }
                }

                auto lambda_return_decl = std::make_unique<function_declaration>(identifier::return_identifier, lambda_function_decl->our_scope.get(), function_declaration::kind::expression_bodied);
                assign_source_information(context, lambda_return_decl, expression);
                lambda_return_decl->body = std::move(chain);

                lambda_function_decl->body = std::move(lambda_return_decl);
            }
        }

        return std::move(lambda_function_decl);
    }

    std::unique_ptr<declaration> build_function_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        auto* const decl = ast->children[ast_idx::function::declaration].get();
        auto* const body = ast->children[ast_idx::function::body].get();

        auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

        auto function_decl = std::make_unique<function_declaration>(identifier(decl->identifier), parent_scope, get_function_kind(body, intrinsic));
        assign_source_information(context, function_decl, decl);

        build_inputs_output(context, decl, *function_decl, output_result, ELEMENT_AST_NODE_FUNCTION);

        const auto scope_bodied = body->type == ELEMENT_AST_NODE_SCOPE;
        const auto expression_bodied = body->type == ELEMENT_AST_NODE_CALL
                                       || body->type == ELEMENT_AST_NODE_LITERAL
                                       || body->type == ELEMENT_AST_NODE_LAMBDA
                                       || body->type == ELEMENT_AST_NODE_ANONYMOUS_BLOCK;

        if (scope_bodied)
        {
            assert(!intrinsic);
            build_scope(context, body, *function_decl, output_result);

            const auto* return_func = function_decl->our_scope->find(identifier::return_identifier, false);
            if (!return_func)
            {
                output_result = log_error(context, context->src_context.get(), decl, log_error_message_code::function_missing_return, function_decl->name.value);
                return nullptr;
            }

            function_decl->body = return_func;
        }
        else if (expression_bodied)
        {
            assert(!intrinsic);
            if (!function_decl->our_scope)
            {
                output_result = log_error(context, context->src_context.get(), body, log_error_message_code::missing_declaration_scope, function_decl->name.value);
                return nullptr;
            }

            deferred_expressions deferred_expressions;
            auto chain = build_expression_chain(context, body, function_decl.get(), deferred_expressions, output_result);
            if (chain->expressions.empty())
            {
                output_result = log_error(context, context->src_context.get(), body, log_error_message_code::expression_chain_cannot_be_empty, function_decl->name.value);
                return nullptr;
            }

            if (deferred_expressions.empty())
                function_decl->body = std::move(chain);
            else
            {
                for (auto& [identifier, expression] : deferred_expressions)
                {

                    auto lambda = build_lambda_declaration(context, identifier, expression, function_decl->our_scope.get(), output_result);
                    const auto is_added = function_decl->our_scope->add_declaration(std::move(lambda));
                    if (!is_added)
                    {
                        //todo: error
                    }
                }

                auto lambda_return_decl = std::make_unique<function_declaration>(identifier::return_identifier, function_decl->our_scope.get(), function_declaration::kind::expression_bodied);
                assign_source_information(context, lambda_return_decl, decl);
                lambda_return_decl->body = std::move(chain);

                function_decl->body = std::move(lambda_return_decl);
            }
        }
        else if (intrinsic && body->type == ELEMENT_AST_NODE_NO_BODY)
        {
            if (intrinsic::register_intrinsic<function_declaration>(context, ast, *function_decl))
                function_decl->body = intrinsic::get_intrinsic(context, *function_decl);
        }
        else
        {
            output_result = log_error(context, context->src_context.get(), decl, log_error_message_code::invalid_function_declaration, function_decl->name.value);
            return nullptr;
        }

        bool had_default = false;
        for (const auto& input : function_decl->inputs)
        {
            if (function_decl->our_scope->find(identifier{ input.get_name() }, false))
                output_result = log_error(context, context->src_context.get(), decl, log_error_message_code::multiple_definition_with_parameter, input.get_name(), function_decl->name.value);

            if (input.has_default())
            {
                had_default = true;
            }
            else if (!input.has_default())
            {
                if (had_default)
                    output_result = log_error(context, context->src_context.get(), decl, log_error_message_code::default_argument_not_at_end, input.get_name(), function_decl->name.value);
            }
        }

        if (output_result != ELEMENT_OK)
            return nullptr;

        return function_decl;
    }

    std::unique_ptr<declaration> build_namespace_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
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

    std::unique_ptr<declaration> build_declaration(const element_interpreter_ctx* context, const element_ast* const ast, const scope* const parent_scope, element_result& output_result)
    {
        std::unique_ptr<declaration> result;
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

    std::unique_ptr<expression> build_literal_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, element_result& output_result)
    {
        const auto is_empty = chain->expressions.empty();
        if (!is_empty)
        {
            output_result = log_error(context, context->src_context.get(), ast, log_error_message_code::invalid_literal_expression_placement);
            return nullptr;
        }

        auto expression = std::make_unique<literal_expression>(ast->literal, chain);
        assign_source_information(context, expression, ast);
        return std::move(expression);
    }

    std::unique_ptr<expression> build_identifier_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, element_result& output_result)
    {
        //no need to test for !chain->expressions.empty() since build_indexing_expression code path will always be taken in this case
        auto expression = std::make_unique<identifier_expression>(ast->identifier, chain);
        assign_source_information(context, expression, ast);
        return std::move(expression);
    }

    std::unique_ptr<expression> build_indexing_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, element_result& output_result)
    {
        //no need to test for chain->expressions.empty() since build_identifier_expression code path will always be taken in this case
        auto expression = std::make_unique<indexing_expression>(ast->identifier, chain);
        assign_source_information(context, expression, ast);
        return std::move(expression);
    }

    std::unique_ptr<expression> build_call_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, deferred_expressions& deferred_expressions, element_result& output_result)
    {
        const auto is_empty = chain->expressions.empty();
        if (is_empty)
        {
            output_result = log_error(context, context->src_context.get(), ast, log_error_message_code::invalid_call_expression_placement);
            return nullptr;
        }

        if (ast->children.empty())
        {
            output_result = log_error(context, context->src_context.get(), ast, log_error_message_code::empty_expression);
            return nullptr;
        }

        auto call_expr = std::make_unique<call_expression>(chain);
        assign_source_information(context, call_expr, ast);

        //every child of the current AST node (EXPRLIST) is the start of another expression_chain, comma separated
        for (const auto& child : ast->children)
        {
            //oof, call expressions... you have to be awkward, don't you?
            auto nested_chain = build_expression_chain(context, child.get(), chain->declarer, deferred_expressions, output_result);
            call_expr->arguments.push_back(std::move(nested_chain));
        }

        return std::move(call_expr);
    }

    std::unique_ptr<expression> build_anonymous_block_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, element_result& output_result)
    {
        assert(chain);
        assert(chain->declarer);
        assert(chain->get_scope());

        if (ast->type != ELEMENT_AST_NODE_ANONYMOUS_BLOCK)
        {
            output_result = ELEMENT_ERROR_UNKNOWN;
            return nullptr;
        }

        auto anonymous_block_expr = std::make_unique<anonymous_block_expression>(chain);
        anonymous_block_expr->our_scope = std::make_unique<scope>(chain->get_scope(), chain);
        build_scope(context, ast, anonymous_block_expr->our_scope.get(), output_result);

        return std::move(anonymous_block_expr);
    }

    std::unique_ptr<expression_chain> build_expression_chain(const element_interpreter_ctx* context, const element_ast* const ast, const declaration* declarer, deferred_expressions& deferred_expressions, element_result& output_result)
    {
        auto chain = std::make_unique<expression_chain>(declarer);
        assign_source_information(context, chain, ast);

        //clean me up, Scotty!

        //early return if lambda
        if (ast->type == ELEMENT_AST_NODE_LAMBDA)
        {

            const auto identifier_string = fmt::format("<{}_{}>", declarer->name.value, deferred_expressions.size());
            auto identifier = element::identifier(identifier_string);
            auto expression = std::make_unique<identifier_expression>(identifier, chain.get());
            assign_source_information(context, expression, ast);
            chain->expressions.push_back(std::move(expression));
            deferred_expressions.push_back({ identifier, ast });
            return std::move(chain);
        }

        if (ast->type == ELEMENT_AST_NODE_ANONYMOUS_BLOCK)
        {

            auto anonymous_block = build_anonymous_block_expression(context, ast, chain.get(), output_result);
            chain->expressions.push_back(std::move(anonymous_block));
            return std::move(chain);
        }

        //start of an expression chain, then build the rest of it
        auto first_expression = build_expression(context, ast, chain.get(), deferred_expressions, output_result);
        assert(first_expression);
        chain->expressions.push_back(std::move(first_expression));

        //every child of the first AST node is part of the chain
        for (const auto& child : ast->children)
        {
            auto chained_expression = build_expression(context, child.get(), chain.get(), deferred_expressions, output_result);
            chain->expressions.push_back(std::move(chained_expression));
        }

        return std::move(chain);
    }

    std::unique_ptr<expression> build_expression(const element_interpreter_ctx* context, const element_ast* const ast, expression_chain* chain, deferred_expressions& deferred_expressions, element_result& output_result)
    {
        assert(chain);

        const auto is_empty = chain->expressions.empty();

        if (ast->type == ELEMENT_AST_NODE_LITERAL)
            return build_literal_expression(context, ast, chain, output_result);

        if (is_empty && ast->type == ELEMENT_AST_NODE_CALL)
            return build_identifier_expression(context, ast, chain, output_result);

        if (!is_empty && ast->type == ELEMENT_AST_NODE_CALL)
            return build_indexing_expression(context, ast, chain, output_result);

        if (ast->type == ELEMENT_AST_NODE_EXPRLIST)
            return build_call_expression(context, ast, chain, deferred_expressions, output_result);

        //todo: error
        output_result = ELEMENT_ERROR_UNKNOWN;
        return nullptr;
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
            const bool success = our_scope->add_declaration(std::move(decl));
            if (!success)
            {
                if (output_result == ELEMENT_OK)
                    output_result = ELEMENT_ERROR_UNKNOWN;
            }
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

        if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_object_model_as_code))
        {
            std::cout << "\n<CODE>\n";
            std::cout << root->to_code();
            std::cout << "\n</CODE>\n\n";
        }

        return root;
    }
} // namespace element
