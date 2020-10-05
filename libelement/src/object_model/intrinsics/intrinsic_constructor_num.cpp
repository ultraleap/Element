#include "intrinsic_constructor_num.hpp"

//SELF
#include "object_model/constraints/type.hpp"

using namespace element;

intrinsic_constructor_num::intrinsic_constructor_num()
    : intrinsic_function(type_id, type::num.get())
{
}

object_const_shared_ptr intrinsic_constructor_num::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    auto expr = std::dynamic_pointer_cast<const element_instruction>(compiled_args[0]);
    assert(expr); //todo: I don't think it could be anything but an expression?
    expr->actual_type = type::num.get();

    return evaluate(context, std::move(expr));
}