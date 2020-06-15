#include <iostream>
#include <memory>

#include "object_model.hpp"
#include "ast/ast_indexes.hpp"
#include "ast/ast_internal.hpp"
#include "declarations/declaration.hpp"
#include "expressions/expression.hpp"

void build_scope(element_ast* ast, const element::scoped_declaration& declaration);

template <class T, class... S> static std::unique_ptr<T> create_type(S&&... args) {
    return std::make_shared<T>(std::forward<S>(args)...);
}

void log(const std::string& message)
{
    //do something better
    std::cout << message << std::endl;
}

void log(element_ast* ast)
{
    log(ast->identifier);
}

void build_output(element_ast* ast, element::declaration& declaration)
{
    const auto has_declared_output = ast->children.size() > ast_idx::declaration::outputs;

    //TODO: JM - Handle complex return with port list and whatnot
    if (has_declared_output) {
        auto* const output = ast->children[ast_idx::declaration::outputs].get();
    }

    //TODO: JM - Use static definition for implicit return?
    declaration.output = std::make_unique<element::port>("return");
    //TODO: JM - Constraints
    //declaration.constraint = std::make_unique<element_constraint>("return");
}

void build_inputs(element_ast* ast, element::declaration& declaration)
{
    auto* const inputs = ast->children[ast_idx::declaration::inputs].get();

    for (auto& input : inputs->children)
        declaration.inputs.emplace_back(input->identifier);
}

std::shared_ptr<element::declaration> element::build_struct_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

    auto struct_decl = std::make_unique<struct_declaration>(parent_scope, intrinsic);
    struct_decl->identifier = decl->identifier;

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
    return nullptr;
}

std::shared_ptr<element::declaration> element::build_function_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const body = ast->children[ast_idx::function::body].get();
    if (body->type == ELEMENT_AST_NODE_SCOPE || body->type == ELEMENT_AST_NODE_CONSTRAINT /* intrinsic function*/)
        return build_scope_bodied_function_declaration(ast, parent_scope);

    if (body->type == ELEMENT_AST_NODE_CALL) 
        return build_expression_bodied_function_declaration(ast, parent_scope);

    return nullptr;
}

std::shared_ptr<element::declaration> element::build_scope_bodied_function_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

    auto function_decl = std::make_unique<function_declaration>(parent_scope, intrinsic);
    function_decl->identifier = decl->identifier;

    build_inputs(decl, *function_decl);
    build_output(decl, *function_decl);

    auto* const body = ast->children[ast_idx::function::body].get();
    if (body->type == ELEMENT_AST_NODE_SCOPE)
        build_scope(body, *function_decl);

    //log(function_decl->to_string());

    return std::move(function_decl);
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

    parent->children.push_back(call_expression);
    return parent;
}

std::shared_ptr<element::expression> element::build_expression(const element_ast* const ast, std::shared_ptr<element::expression> parent)
{
    //HC SVNT DRACONES
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

    //indexing shouldn't return early, as it *might* be followed by a call expression,
    //since indexing and call expressions are combined in the ast stage
    const auto is_indexing = has_parent
        && (ast->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_CALL || ast->children[ast_idx::call::parent]->type == ELEMENT_AST_NODE_LITERAL);

	if (is_indexing)
    {
	    auto indexing_expression = std::make_unique<element::indexing_expression>(ast->identifier, parent->enclosing_scope);
	    parent->children.push_back(std::move(indexing_expression));
    }
    else
    {
	    const auto identifier_expression = std::make_shared<element::identifier_expression>(ast->identifier, parent->enclosing_scope);
	    parent->children.push_back(identifier_expression);
    }

	if(has_arguments)
        return build_call_expression(ast, std::move(parent));
	
    return parent;
}

std::shared_ptr<element::declaration> element::build_expression_bodied_function_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);
    assert(!intrinsic);

    auto function_decl = std::make_unique<element::expression_bodied_function_declaration>(parent_scope);
    function_decl->identifier = decl->identifier;

    build_inputs(decl, *function_decl);
    build_output(decl, *function_decl);

    auto* const body = ast->children[ast_idx::function::body].get();
    if (body->type == ELEMENT_AST_NODE_CALL) {

        auto expression = std::make_unique<element::expression>(function_decl->scope.get());
        function_decl->expression = build_expression(body, std::move(expression));
    }
	
    //log(function_decl->to_string());

    return std::move(function_decl);
}

std::shared_ptr<element::declaration> element::build_namespace_declaration(const element_ast* const ast, const scope* const parent_scope)
{
    auto namespace_decl = std::make_unique<namespace_declaration>(parent_scope);
    namespace_decl->identifier = ast->identifier;

    //log(namespace_decl->to_string());

    if (ast->children.size() > ast_idx::ns::body)
    {
        auto* body = ast->children[ast_idx::ns::body].get();
        if (body->type == ELEMENT_AST_NODE_SCOPE)
            build_scope(body, *namespace_decl);
    }

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

void build_scope(element_ast* ast, const element::scoped_declaration& declaration)
{
    for (auto& child : ast->children)
    {
        auto child_decl = build_declaration(child.get(), declaration.scope.get());
        if (child_decl)
            declaration.add_declaration(std::move(child_decl));
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