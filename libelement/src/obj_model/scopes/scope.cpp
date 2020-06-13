#include "scope.hpp"

std::string element::scope::to_string() const
{
	return declarer->to_string();
}

std::string element::scope::location() const
{
	return declarer->location();
}

void element::scope::add_declaration(std::shared_ptr<element::declaration> declaration)
{
    declarations.emplace(declaration->identifier, std::move(declaration));
}

std::shared_ptr<element::declaration> element::scope::find(const std::string& identifier, const bool recurse = false) const
{
    const auto name_it = declarations.find(identifier);
    if (name_it != declarations.end())
            return name_it->second;

    return (recurse && parent_scope) ? parent_scope->find(identifier, true) : nullptr;
}