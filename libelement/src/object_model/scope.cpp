#include "scope.hpp"

//SELF
#include "object_model/declarations/declaration.hpp"
#include "object_model/expressions/expression_chain.hpp"
#include "object_model/error.hpp"

using namespace element;

scope::scope(const scope* parent_scope, const object* const declaration_or_expression)
    : /*declaration_or_expression(declaration_or_expression)
    ,*/
    parent_scope(parent_scope)
{
}

element_result scope::merge(std::unique_ptr<scope>&& other)
{
    if (!other->is_root())
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

bool scope::mark_declaration_compiler_generated(const identifier& name)
{
    const auto found_it = declarations.find(name);
    if (found_it == declarations.end())
        return false;

    auto decl = std::move(found_it->second);
    decl->name.value = "@" + name.value;
    declarations.erase(found_it);
    auto [it, success] = declarations.try_emplace(decl->name, std::move(decl));
    return success;
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

bool scope::remove_declaration(const identifier& name)
{
    return declarations.erase(name.value) != 0;
}

const std::map<identifier, std::unique_ptr<declaration>>& scope::get_declarations() const
{
    return declarations;
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