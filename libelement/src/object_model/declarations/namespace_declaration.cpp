#include "namespace_declaration.hpp"

//SELF
#include "object_model/scope.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"

using namespace element;

namespace_declaration::namespace_declaration(identifier name, const scope* parent_scope)
    : declaration(std::move(name), parent_scope)
{
    qualifier = namespace_qualifier;
}

object_const_shared_ptr namespace_declaration::index(
    const compilation_context& context,
    const identifier& name,
    const source_information& source_info) const
{
    if (our_scope)
    {
        const auto* found = our_scope->find(name, context.interpreter->caches, false);
        if (found)
            return found->compile(context, source_info);

        return build_error_and_log<error_message_code::failed_to_find_when_resolving_indexing_expr>(context, source_info, name.value, to_string());
    }

    throw;
}

bool namespace_declaration::is_intrinsic() const
{
    return false;
}