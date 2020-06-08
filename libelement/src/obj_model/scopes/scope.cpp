#include "scope.hpp"

std::string element::scope::to_string() const
{
	return declarer->to_string();
}

void element::scope::add_declaration(std::unique_ptr< element::declaration> declaration)
{
    declarations.push_back(std::move(declaration));
}