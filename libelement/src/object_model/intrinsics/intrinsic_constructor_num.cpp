#include "intrinsic_constructor_num.hpp"

//SELF
#include "object_model/constraints/type.hpp"
#include "object_model/error.hpp"

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
    auto expr = std::dynamic_pointer_cast<const instruction>(compiled_args[0]);

    if (!expr)
        return std::make_shared<const element::error>(fmt::format("Argument to Num was '{}' which is invalid. Must be Num or Bool.", compiled_args[0]->to_string()), ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);

    expr->actual_type = type::num.get();

    return evaluate(context, std::move(expr));
}