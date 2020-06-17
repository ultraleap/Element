#include "object_model.hpp"

//STD
#include <iostream>
#include <memory>

//SELF
#include "object.hpp"
#include "ast/ast_indexes.hpp"
#include "ast/ast_internal.hpp"
#include "declarations.hpp"
#include "expressions.hpp"
#include "configuration.hpp"
#include "functions.hpp"

void build_scope(element_ast* ast, const element::declaration& declaration);

void log(const std::string& message)
{
    //do something better
    std::cout << message << std::endl;
}

void log(element_ast* ast)
{
    log(ast->identifier);
}

std::unique_ptr<element::type_annotation> build_type_annotation(const element_ast* ast)
{
    if (ast->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE)
        return nullptr;

    if (ast->type == ELEMENT_AST_NODE_TYPENAME) 
    {
        auto* const identifier = ast->children[ast_idx::port::type].get();
        if (identifier->type != ELEMENT_AST_NODE_IDENTIFIER)
            throw;

        return std::make_unique<element::type_annotation>(element::identifier(identifier->identifier));
    }
    
    throw;
}

void build_output(element_ast* ast, element::declaration& declaration)
{
    auto* const output = ast->children[ast_idx::declaration::outputs].get();
    auto type_annotation = build_type_annotation(output);
    declaration.output.emplace(element::port{ element::identifier::return_identifier, std::move(type_annotation) });
}

void build_inputs(element_ast* ast, element::declaration& declaration)
{
    auto* const inputs = ast->children[ast_idx::declaration::inputs].get();

    for (auto& input : inputs->children) 
    {
        if (input->type != ELEMENT_AST_NODE_PORT)
            throw;

        auto identifier = element::identifier(input->identifier);

        const auto has_type_annotation = input->children.size() > ast_idx::port::type;
        if (!has_type_annotation) 
        {
            declaration.inputs.emplace_back(identifier, nullptr);
            continue;
        }

        auto* const output = input->children[ast_idx::port::type].get();
        auto type_annotation = build_type_annotation(output);
        declaration.inputs.emplace_back(identifier, std::move(type_annotation));
    }
}

std::shared_ptr<element::declaration> element::build_struct_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

    auto struct_decl = std::make_unique<struct_declaration>(identifier(decl->identifier), parent_scope, intrinsic);

    //fields
    build_inputs(decl, *struct_decl);
    build_output(decl, *struct_decl);

    //log(struct_decl->to_string());

    if (ast->children.size() > ast_idx::function::body)
    {
        if (ast->children.size() > ast_idx::function::body)
        {
            auto* body = ast->children[ast_idx::function::body].get();
            if (body->type == ELEMENT_AST_NODE_SCOPE)
                build_scope(body, *struct_decl);
        }
    }

    return std::move(struct_decl);
}

std::shared_ptr<element::declaration> element::build_constraint_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

    auto constraint_decl = std::make_unique<constraint_declaration>(identifier(decl->identifier), intrinsic);

    //ports
    build_inputs(decl, *constraint_decl);
    build_output(decl, *constraint_decl);

    //log(constraint_decl->to_string());

    return std::move(constraint_decl);
}

std::shared_ptr<element::declaration> element::build_function_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

    auto function_decl = std::make_shared<function_declaration>(identifier(decl->identifier), parent_scope, intrinsic);

    build_inputs(decl, *function_decl);
    build_output(decl, *function_decl);

    auto* const body = ast->children[ast_idx::function::body].get();

    if (body->type == ELEMENT_AST_NODE_SCOPE)
    {
        assert(!intrinsic);
        build_scope(body, *function_decl);
        function_decl->body = function_decl->our_scope->find("return", false);
    }
    else if (body->type == ELEMENT_AST_NODE_CALL || body->type == ELEMENT_AST_NODE_LITERAL)
    {
        assert(!intrinsic);
        auto expression = std::make_unique<element::expression>(function_decl->our_scope.get());
        function_decl->body = build_expression(body, std::move(expression));
    }
    else if (body->type == ELEMENT_AST_NODE_CONSTRAINT)
    {
        assert(intrinsic);
        function_decl->body = intrinsic::get_intrinsic(*function_decl);
    }
    else
    {
        function_decl = nullptr;
    }

    return function_decl;
}

[[nodiscard]] std::shared_ptr<element::expression> build_literal_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent)
{
	const auto literal_expression = std::make_shared<element::literal_expression>(ast->literal, parent->enclosing_scope);
    parent->children.push_back(literal_expression);
    return parent;
}

[[nodiscard]] std::shared_ptr<element::expression> build_call_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent)
{
	//create call expression to represent the nested expression
    const auto call_expression = std::make_shared<element::call_expression>(parent->enclosing_scope);
	
    auto* const expressions = ast->children[ast_idx::call::args].get();
    for (auto& child_expression : expressions->children) {

    	//then create an expression for each comma separated value in the call expression 
        const auto expression = std::make_shared<element::expression>(parent->enclosing_scope);
        auto child = build_expression(child_expression.get(), expression);
        call_expression->children.push_back(child);
    }

    //auto body = element::build_function_declaration(ast, parent);

    parent->children.push_back(call_expression);
    return parent;
}

std::shared_ptr<element::expression> build_scope_bodied_lambda_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent_scope)
{

    return nullptr;
}

std::shared_ptr<element::expression> build_expression_bodied_lambda_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent_scope)
{

    return nullptr;
}

[[nodiscard]] std::shared_ptr<element::expression> build_lambda_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent)
{
    //log("LAMBDA NOT IMPLEMENTED");
    return nullptr;
}

std::shared_ptr<element::expression> element::build_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent)
{
    const auto has_parent =
        ast->children.size() > ast_idx::call::parent && ast->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;

	const auto has_arguments = 
        ast->children.size() > ast_idx::call::args && ast->children[ast_idx::call::args]->type == ELEMENT_AST_NODE_EXPRLIST;

    //recurse up to parent to reverse ast definition
    if (has_parent) 
        parent = build_expression(ast->children[ast_idx::call::parent].get(), parent);

    const auto is_literal = ast->type == ELEMENT_AST_NODE_LITERAL;
    if (is_literal)
        return build_literal_expression(ast, std::move(parent));


    //const auto is_lambda = ast->type == ELEMENT_AST_NODE_LAMBDA;
    //if (is_lambda)
    //    return build_lambda_expression(ast, std::move(parent));

    //indexing shouldn't return early, as it *might* be followed by a call expression,
    //since indexing and call expressions are combined in the ast stage
    const auto is_indexing = has_parent
        && (ast->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_CALL || ast->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_LITERAL);

	if (is_indexing)
    {
	    auto indexing_expression = std::make_unique<element::indexing_expression>(identifier(ast->identifier), parent->enclosing_scope);
	    parent->children.push_back(std::move(indexing_expression));
    }
    else
    {
	    const auto identifier_expression = std::make_shared<element::identifier_expression>(identifier(ast->identifier), parent->enclosing_scope);
	    parent->children.push_back(identifier_expression);
    }

	if(has_arguments)
        return build_call_expression(ast, std::move(parent));
	
    return parent;
}

std::shared_ptr<element::declaration> element::build_namespace_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto namespace_decl = std::make_unique<namespace_declaration>(identifier(ast->identifier), parent_scope);

    if (ast->children.size() > ast_idx::ns::body)
    {
        auto* body = ast->children[ast_idx::ns::body].get();
        if (body->type == ELEMENT_AST_NODE_SCOPE)
            build_scope(body, *namespace_decl);
    }

    //log(namespace_decl->to_string());

    return std::move(namespace_decl);
}

std::shared_ptr<element::declaration> element::build_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    if (ast->type == ELEMENT_AST_NODE_STRUCT)
        return build_struct_declaration(ast, parent_scope);

    if (ast->type == ELEMENT_AST_NODE_CONSTRAINT)
        return build_constraint_declaration(ast, parent_scope);

    if (ast->type == ELEMENT_AST_NODE_FUNCTION)
        return build_function_declaration(ast, parent_scope);

    if (ast->type == ELEMENT_AST_NODE_NAMESPACE)
        return build_namespace_declaration(ast, parent_scope);

    //log("Not a declaration");
    return nullptr;
}

void build_scope(element_ast* ast, const element::declaration& declaration)
{
    for (auto& child : ast->children)
    {
        auto child_decl = build_declaration(child.get(), declaration.our_scope.get());
        if (child_decl)
            declaration.our_scope->add_declaration(std::move(child_decl));
    }
}

std::unique_ptr<element::scope> element::build_root_scope(const element_ast* const ast)
{
    if (ast->type != ELEMENT_AST_NODE_ROOT) {

        //log("Not a root");
        return nullptr;
    }

    auto root = std::make_unique<element::scope>(nullptr, nullptr);

    for (auto& child : ast->children)
    {
        auto decl = build_declaration(child.get(), root.get());
        if (decl)
            root->add_declaration(std::move(decl));
    }

    if (flag_set(logging_bitmask, log_flags::debug | log_flags::output_object_model_as_code)) {
        log("\n<CODE>");
        log(root->to_code());
        log("</CODE>\n");
    }

    return root;
}