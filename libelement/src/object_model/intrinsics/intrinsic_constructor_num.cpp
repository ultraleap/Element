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

    //Instructions always resolve to either a bool or a float, and we only support converting from one to the other
    if (!expr) {
        return std::make_shared<const element::error>(
            fmt::format("Argument to intrinsic 'Num' was '{}' which is invalid. Must be 'Num' or 'Bool'",
                compiled_args[0]->to_string()), ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);
    }

    //If it's already a number, we don't need to do anything
    if (expr->actual_type == type::num.get())
        return expr;

    //If it's a bool, we can wrap it in a IF to convert it to a number
    if (expr->actual_type == type::boolean.get()) {
        auto new_expr = context.interpreter->cache_instruction_if.get(
            expr,
            context.interpreter->cache_instruction_constant.get(1, type::num.get()),
            context.interpreter->cache_instruction_constant.get(0, type::num.get()));
        
        return evaluate(context, std::move(new_expr));
    }

    //If we screwed up somewhere, the type info might be missing
    if (!expr->actual_type) {
        return std::make_shared<const element::error>(
            fmt::format("Argument to intrinsic 'Num' was '{}' which has null type information. Report to developers",
                expr->to_string()), ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);
    }

    //If we really screwed up somewhere, the type info might be wrong
    return std::make_shared<const element::error>(
        fmt::format("Argument to intrinsic 'Num' was '{}' which has unknown type information. Report to developers",
            expr->to_string()), ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);
}

std::string intrinsic_constructor_num::get_name() const
{
    return "Num";
}
