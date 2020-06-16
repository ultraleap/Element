#include "object.hpp"
#include "scope.hpp"

element::identifier element::identifier::unidentifier{"_"};

element::compilation_context::compilation_context(const scope* const scope): global_scope{scope}
{
}
