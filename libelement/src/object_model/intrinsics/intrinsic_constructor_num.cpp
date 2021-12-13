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

    //If it's not an instruction then we definitely can't convert it, and if it's not a Num/Bool type then it's not valid.
    if (!expr || !expr->actual_type) {
        return std::make_shared<const element::error>(
            fmt::format("Argument to intrinsic 'Num' was '{}' which is invalid. Must be 'Num' or 'Bool'",
                compiled_args[0]->to_string()),
            ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);
    }

    //If it's already a Num, we don't need to do anything
    if (expr->actual_type == type::num.get())
        return expr;

    //If it's a Bool, we can wrap it in a IF to convert it to a number
    if (expr->actual_type == type::boolean.get()) {
        auto new_expr = context.interpreter->cache_instruction_if.get(
            expr,
            context.interpreter->cache_instruction_constant.get(1.0f, type::num.get()),
            context.interpreter->cache_instruction_constant.get(0.0f, type::num.get()));

        return evaluate(context, std::move(new_expr));
    }

    //If we really screwed up somewhere, the type info might be wrong
    return std::make_shared<const element::error>(
        fmt::format("Argument to intrinsic 'Num' was '{}' which has unknown type information. Report to developers",
            expr->to_string()),
        ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);
}

std::string intrinsic_constructor_num::get_name() const
{
    return "Num";
}
