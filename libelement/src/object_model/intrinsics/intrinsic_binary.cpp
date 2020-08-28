#include "intrinsic_binary.hpp"

//SELF
#include "object_model/constraints/type.hpp"
#include "object_model/declarations/declaration.hpp"
#include "object_model/error.hpp"

using namespace element;

intrinsic_binary::intrinsic_binary(const element_binary_op operation, type_const_ptr return_type = type::num.get(),
                                    type_const_ptr first_argument_type = type::num.get(),
                                    type_const_ptr second_argument_type = type::num.get()):
    intrinsic_function(type_id, return_type)
    , operation(operation)
    , first_argument_type{first_argument_type}
    , second_argument_type{second_argument_type}
{
}

object_const_shared_ptr intrinsic_binary::compile(const compilation_context& context,
                                                  const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *frame.function;
    assert(declarer.inputs.size() == 2);
    assert(frame.compiled_arguments.size() == 2);

    const auto intrinsic = get_intrinsic(context.interpreter, declarer);
    assert(intrinsic);
    assert(intrinsic == this);

    auto expr1_err = std::dynamic_pointer_cast<const error>(frame.compiled_arguments[0]);
    if (expr1_err)
        return expr1_err;

    auto expr2_err = std::dynamic_pointer_cast<const error>(frame.compiled_arguments[1]);
    if (expr2_err)
        return expr2_err;

    auto expr1 = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
    auto expr2 = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[1]);
    assert(expr1);
    assert(expr2);

    auto new_expr = std::make_unique<element_expression_binary>(
        operation,
        expr1,
        expr2,
        return_type);

    return evaluate(context, std::move(new_expr));
}