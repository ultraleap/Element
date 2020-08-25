#include "intrinsic_constructor_bool.hpp"

//SELF
#include "object_model/constraints/type.hpp"

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
    auto& true_decl = *context.get_global_scope()->find(identifier("True"), false);
    auto& false_decl = *context.get_global_scope()->find(identifier("False"), false);

    const auto true_expr = get_intrinsic(context.interpreter, true_decl)->compile(context, source_info);
    const auto false_expr = get_intrinsic(context.interpreter, false_decl)->compile(context, source_info);

    auto expr = std::dynamic_pointer_cast<const element_expression>(compiled_args[0]);
    
    assert(expr); //todo: I think this is accurate
    assert(std::dynamic_pointer_cast<const element_expression>(true_expr));
    assert(std::dynamic_pointer_cast<const element_expression>(false_expr));

    auto new_expr = std::make_unique<element_expression_if>(
        expr,
        std::dynamic_pointer_cast<const element_expression>(true_expr),
        std::dynamic_pointer_cast<const element_expression>(false_expr));

    return evaluate(context, std::move(new_expr));
}