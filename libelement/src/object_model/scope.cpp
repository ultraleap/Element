#include "scope.hpp"

//SELF
#include "object_model/declarations/declaration.hpp"
#include "object_model/expressions/expression_chain.hpp"

using namespace element;

scope::scope(const scope* parent_scope, const object* const declaration_or_expression)
    : /*declaration_or_expression(declaration_or_expression)
    ,*/ parent_scope(parent_scope)
{
}

element_result scope::merge(std::unique_ptr<scope>&& other)
{
    if (!is_root() || !other->is_root())
        return ELEMENT_ERROR_UNKNOWN;

    for (auto& [identifier, declaration] : other->declarations)
    {
        if (declarations.count(identifier))
            return ELEMENT_ERROR_MULTIPLE_DEFINITIONS;

        declarations[identifier] = std::move(declaration);
        if (declarations[identifier]->our_scope)
            declarations[identifier]->our_scope->parent_scope = this;
    }

    return ELEMENT_OK;
}

//std::string scope::location() const
//{
//    if (!declaration_or_expression)
//        return "Not available";
//
//    if (const auto* decl = dynamic_cast<const declaration*>(declaration_or_expression))
//    {
//        return decl->location();
//    }
//
//    //todo: location
//    if (const auto* expr = dynamic_cast<const expression*>(declaration_or_expression))
//    {
//        return *expr->source_info.line_in_source;
//    }
//
//    if (const auto* expr_chain = dynamic_cast<const expression_chain*>(declaration_or_expression))
//    {
//        return *expr_chain->source_info.line_in_source;
//    }
//
//    assert(false);
//    throw;
//    //return declaration_or_expression ? declaration_or_expression->location() : "Not available";
//}

bool scope::add_declaration(std::unique_ptr<declaration> declaration)
{
    const auto& [it, success] = declarations.try_emplace(declaration->name.value, std::move(declaration));
    return success;
}

const declaration* scope::find(const identifier& name, const bool recurse = false) const
{
    const auto name_it = declarations.find(name);
    if (name_it != declarations.end())
        return name_it->second.get();

    return (recurse && parent_scope) ? parent_scope->find(name, true) : nullptr;
}

const scope* scope::get_global() const
{
    const auto* global = this;
    while (global->parent_scope)
        global = global->parent_scope;

    return global;
}