#include "intrinsic_unary.hpp"

//SELF
#include "object_model/constraints/type.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "instruction_tree/instructions.hpp"

using namespace element;

intrinsic_unary::intrinsic_unary(element_unary_op operation,
                                 type_const_ptr return_type = type::num.get(),
                                 type_const_ptr argument_type = type::num.get())
    : intrinsic_function(type_id, return_type)
    , operation(operation)
    , argument_type{ argument_type }
{
}

object_const_shared_ptr intrinsic_unary::compile(const compilation_context& context,
                                                 const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *static_cast<const declaration*>(frame.function->declarer);
    assert(declarer.inputs.size() == 1);
    assert(frame.compiled_arguments.size() == 1);

    const auto* intrinsic = get_intrinsic(context.interpreter, declarer);
    assert(intrinsic);
    assert(intrinsic == this);

    auto expr = std::dynamic_pointer_cast<const instruction>(frame.compiled_arguments[0]);
    assert(expr);

    auto new_expr = std::make_unique<element::instruction_unary>(
        operation,
        std::move(expr),
        return_type);

    return evaluate(context, std::move(new_expr));
}