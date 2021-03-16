#include "intrinsic_constructor_bool.hpp"

//SELF
#include "object_model/constraints/type.hpp"
#include "object_model/error.hpp"

using namespace element;

intrinsic_constructor_bool::intrinsic_constructor_bool()
    : intrinsic_function(type_id, type::boolean.get())
{
}

object_const_shared_ptr intrinsic_constructor_bool::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    auto& true_decl = *context.get_global_scope()->find(identifier("True"), context.interpreter->caches, false);
    auto& false_decl = *context.get_global_scope()->find(identifier("False"), context.interpreter->caches, false);

    const auto true_expr = get_intrinsic(context.interpreter, true_decl)->compile(context, source_info);
    const auto false_expr = get_intrinsic(context.interpreter, false_decl)->compile(context, source_info);

    auto expr = std::dynamic_pointer_cast<const instruction>(compiled_args[0]);

    if (!expr)
        return std::make_shared<const element::error>(fmt::format("Argument to Bool was '{}' which is invalid. Must be Num or Bool.", compiled_args[0]->to_string()), ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);

    assert(std::dynamic_pointer_cast<const instruction>(true_expr));
    assert(std::dynamic_pointer_cast<const instruction>(false_expr));

    auto new_expr = std::make_unique<element::instruction_if>(
        expr,
        std::dynamic_pointer_cast<const instruction>(true_expr),
        std::dynamic_pointer_cast<const instruction>(false_expr));

    return evaluate(context, std::move(new_expr));
}

std::string intrinsic_constructor_bool::get_name() const
{
    return "Bool";
}
