#include "intrinsic_for.hpp"

//SELF
#include "object_model/error.hpp"

using namespace element;

intrinsic_for::intrinsic_for()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr intrinsic_for::compile(const compilation_context& context,
                                              const source_information& source_info) const
{
    return nullptr;
}