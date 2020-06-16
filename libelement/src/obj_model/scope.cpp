#include "scope.hpp"

namespace element
{
std::string scope::to_string() const
{
    return declarer->to_string();
}

std::string scope::to_code(int depth) const
{
    std::string code;

    const std::string offset = "    ";
    std::string scope_offset;

    for (auto i = 0; i < depth; ++i)
        scope_offset += offset;

    const auto end = std::end(declarations);
    for (auto declaration = std::begin(declarations); declaration != end; ++declaration)
        code += declaration->second->to_code(depth + 1) + (std::next(declaration) == end ? "" : "\n");

    if (is_root())
        return code;

    return "\n" + scope_offset + "{\n" + code + "\n" + scope_offset + "}";
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

std::string scope::location() const
{
    return declarer->location();
}

scope::scope(const scope* parent_scope, const declaration* const declarer)
    : declarer(declarer)
    , parent_scope(parent_scope)
{
}

void scope::add_declaration(std::shared_ptr<declaration> declaration)
{
    declarations.emplace(declaration->name.value, std::move(declaration));
}

std::shared_ptr<declaration> scope::find(const std::string& name, const bool recurse = false) const
{
    const auto name_it = declarations.find(name);
    if (name_it != declarations.end())
        return name_it->second;

    return (recurse && parent_scope) ? parent_scope->find(name, true) : nullptr;
}

const scope* scope::get_global() const
{
    //const static scope* global = nullptr;
    //if (global)
    //    return global;

    const auto* local = this;
    while (local->parent_scope)
        local = local->parent_scope;

    //global = local;
    return local;
}
}