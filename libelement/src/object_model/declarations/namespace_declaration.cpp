#include "namespace_declaration.hpp"

//SELF
#include "../scope.hpp"

using namespace element;

namespace_declaration::namespace_declaration(identifier name, const scope* parent_scope)
    : declaration(std::move(name), parent_scope)
{
    qualifier = namespace_qualifier;
    _intrinsic = false;
}

object_const_shared_ptr namespace_declaration::index(
    const compilation_context& context,
    const identifier& name,
    const source_information& source_info) const
{
    return our_scope->find(name, false)->compile(context, source_info);
}