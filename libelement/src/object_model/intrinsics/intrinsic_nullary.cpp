#include "intrinsic_nullary.hpp"

//SELF
#include "object_model/constraints/type.hpp"

using namespace element;

intrinsic_nullary::intrinsic_nullary(const element_nullary_op operation, type_const_ptr return_type = type::num.get())
    : intrinsic_function(type_id, return_type)
    , operation(operation)
{
}

object_const_shared_ptr intrinsic_nullary::compile(const compilation_context& context,
    const source_information& source_info) const
{
    return context.interpreter->cache_instruction_nullary.get(operation, return_type);
}
