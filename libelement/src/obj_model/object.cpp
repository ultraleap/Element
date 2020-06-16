#include "object.hpp"
#include "scope.hpp"

namespace element
{
    identifier identifier::return_identifier{ "return" };

    compilation_context::compilation_context(const scope* const scope) : global_scope{ scope }
    {

    }
}