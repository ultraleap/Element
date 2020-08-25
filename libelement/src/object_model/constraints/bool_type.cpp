#include "bool_type.hpp"

//SELF
#include "object_model/compilation_context.hpp"
#include "object_model/declarations/declaration.hpp"

using namespace element;

object_const_shared_ptr bool_type::index(const compilation_context& context, const identifier& name,
                                         const source_information& source_info) const
{
    //todo: cache, but don't use static
    const auto* declaration = context.get_global_scope()->find(bool_type::name, false);
    assert(declaration);
    return declaration->index(context, name, source_info);
}