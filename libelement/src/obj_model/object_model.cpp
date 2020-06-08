#include <iostream>
#include <memory>

#include "object_model.hpp"
#include "ast/ast_indexes.hpp"
#include "ast/ast_internal.hpp"
#include "declarations/declaration.hpp"

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
    auto* const inputs = ast->children[ast_idx::declaration::outputs].get();

	//TODO: Handle complex return with port list and whatnot

	//TODO: Use static definition for implicit return?
    declaration.output = std::make_unique<element::port>("return");
}

void build_inputs(element_ast* ast, element::declaration& declaration)
{
    auto* const inputs = ast->children[ast_idx::declaration::inputs].get();

    for (auto& input : inputs->children)
        declaration.inputs.emplace_back(input->identifier);
}

std::unique_ptr<element::declaration> element::build_struct_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
	auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

	auto struct_decl = std::make_unique<struct_declaration>(parent_scope, intrinsic);
    struct_decl->identifier = decl->identifier;

    build_inputs(decl, *struct_decl);
    build_output(decl, *struct_decl);

    log(struct_decl->to_string());

    if (ast->children.size() > ast_idx::function::body)
    {
        if (ast->children.size() > ast_idx::function::body)
        {
            auto body = ast->children[ast_idx::function::body].get();
            if (body->type == ELEMENT_AST_NODE_SCOPE)
                build_scope(body, *struct_decl);
        }
    }

    return struct_decl;
}

std::unique_ptr<element::declaration> element::build_constraint_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
    return nullptr;
}

void build_function_body(element_ast* ast)
{
    const auto has_parent =
        ast->children.size() > ast_idx::call::parent && ast->children[ast_idx::call::parent]->type != ELEMENT_AST_NODE_NONE;

    const auto has_arguments = ast->children.size() > ast_idx::call::args
        && ast->children[ast_idx::call::args]->type != ELEMENT_AST_NODE_NONE;

    const auto is_indexing = ast->parent && ast->parent->type == ELEMENT_AST_NODE_CALL;
	
	//recurse
	if(has_parent)
        build_function_body(ast->children[ast_idx::call::parent].get());

    switch (ast->type)
    {
    case ELEMENT_AST_NODE_CONSTRAINT:
        log("CONSTRAINT"); //ONLY IF INTRINSIC == true
        break;
    case ELEMENT_AST_NODE_LITERAL:
        log("LITERAL");
    	
    	if(is_indexing)
            log("INDEXING");
        break;
    case ELEMENT_AST_NODE_CALL:
    	if(has_arguments) //call
			log("CALL");
    	
        if(is_indexing) //then index result of call
            log("INDEXING");
        break;
    case ELEMENT_AST_NODE_SCOPE:
        log("SCOPE");
        break;
    default:
        log("???");
    }
}

std::unique_ptr<element::declaration> element::build_function_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
	//HC SVNT DRACONES
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

	auto function_decl = std::make_unique<function_declaration>(parent_scope, intrinsic);
    function_decl->identifier = decl->identifier;

    build_inputs(decl, *function_decl);
    build_output(decl, *function_decl);

    log(function_decl->to_string());

    auto* const body = ast->children[ast_idx::function::body].get();
    if (body->type == ELEMENT_AST_NODE_SCOPE)
        build_scope(body, *function_decl);
	
    if (body->type == ELEMENT_AST_NODE_CALL)
		build_function_body(body);
	
    return function_decl;
}

std::unique_ptr<element::declaration> element::build_namespace_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
	auto namespace_decl = std::make_unique<namespace_declaration>(parent_scope);
    namespace_decl->identifier = ast->identifier;

    log(namespace_decl->to_string());

    if (ast->children.size() > ast_idx::ns::body)
    {
        auto body = ast->children[ast_idx::ns::body].get();
    	if(body->type == ELEMENT_AST_NODE_SCOPE)
			build_scope(body, *namespace_decl);
    }

    return namespace_decl;
}

std::unique_ptr<element::declaration> element::build_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
    if (ast->type == ELEMENT_AST_NODE_STRUCT)
        return build_struct_declaration(ast, parent_scope);

    if (ast->type == ELEMENT_AST_NODE_CONSTRAINT)
        return build_constraint_declaration(ast, parent_scope);

    if (ast->type == ELEMENT_AST_NODE_FUNCTION)
        return build_function_declaration(ast, parent_scope);

    if (ast->type == ELEMENT_AST_NODE_NAMESPACE)
        return build_namespace_declaration(ast, parent_scope);

    log("Not a declaration");
    return nullptr;
}

void build_scope(element_ast* ast, const element::scoped_declaration& declaration)
{
    for (auto& child : ast->children)
    {
        auto child_decl = build_declaration(child.get(), declaration.scope);
        if (child_decl)
            declaration.add_declaration(std::move(child_decl));
    }
}

std::shared_ptr<element::root_scope> element::build_root_scope(element_ast* ast)
{
    if (ast->type != ELEMENT_AST_NODE_ROOT) {

        log("Not a root");
        return nullptr;
    }

    auto root = std::make_shared<element::root_scope>();

    for (auto& child : ast->children)
    {
        auto decl = build_declaration(child.get(), root);
        if (decl)
            root->add_declaration(std::move(decl));
    }

    return root;
}