#include <iostream>
#include <memory>

#include "object_model.hpp"
#include "ast/ast_indexes.hpp"
#include "ast/ast_internal.hpp"
#include "declarations/declaration.hpp"

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

std::unique_ptr<element::declaration> element::build_struct_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
	auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

	auto struct_decl = std::make_unique<struct_declaration>(parent_scope, intrinsic);
    struct_decl->identifier = decl->identifier;

    log(struct_decl->to_string());

    if (ast->children.size() > ast_idx::function::body)
    {
         auto body = ast->children[ast_idx::function::body].get();
         for (auto& child : body->children)
         {
             auto child_decl = build_declaration(child.get(), struct_decl->scope);
             if (child_decl)
                 struct_decl->add_declaration(std::move(child_decl));
         }
    }

    return struct_decl;
}

std::unique_ptr<element::declaration> element::build_constraint_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
    return nullptr;
}

std::unique_ptr<element::declaration> element::build_function_declaration(element_ast* ast, const std::shared_ptr<element::scope>& parent_scope)
{
	//HC SVNT DRACONES
    auto* const decl = ast->children[ast_idx::function::declaration].get();
    auto intrinsic = decl->has_flag(ELEMENT_AST_FLAG_DECL_INTRINSIC);

	auto function_decl = std::make_unique<function_declaration>(parent_scope, intrinsic);
    function_decl->identifier = decl->identifier;

    log(function_decl->to_string());

    auto* const body = ast->children[ast_idx::function::body].get();
	switch(body->type)
	{
    case ELEMENT_AST_NODE_CONSTRAINT:
        //log("CONSTRAINT"); //ONLY IF INTRINSIC == true
        break;
    case ELEMENT_AST_NODE_LITERAL:
        //log("LITERAL");
        break;
    case ELEMENT_AST_NODE_CALL:
        //log("CALL");
        break;
    case ELEMENT_AST_NODE_SCOPE:
        //log("SCOPE");
        break;
    default:
        log("???");
        return nullptr;
	}
	
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
        for (auto& child : body->children)
        {
            auto child_decl = build_declaration(child.get(), namespace_decl->scope);
            if (child_decl)
                namespace_decl->add_declaration(std::move(child_decl));
        }
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