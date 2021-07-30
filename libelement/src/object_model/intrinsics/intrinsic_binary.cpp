#include "intrinsic_binary.hpp"

//SELF
#include "object_model/constraints/type.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/declarations/function_declaration.hpp"

using namespace element;

intrinsic_binary::intrinsic_binary(const element_binary_op operation, type_const_ptr return_type = type::num.get(),
    type_const_ptr first_argument_type = type::num.get(),
    type_const_ptr second_argument_type = type::num.get())
    : intrinsic_function(type_id, return_type)
    , operation(operation)
    , first_argument_type{ first_argument_type }
    , second_argument_type{ second_argument_type }
{
}

object_const_shared_ptr intrinsic_binary::compile(const compilation_context& context,
    const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *static_cast<const declaration*>(frame.function->declarer);
    assert(declarer.inputs.size() == 2);
    assert(frame.compiled_arguments.size() == 2);

    const auto intrinsic = get_intrinsic(context.interpreter, declarer);
    assert(intrinsic);
    assert(intrinsic == this);

    if (frame.compiled_arguments[0]->is_error())
        return frame.compiled_arguments[0];

    if (frame.compiled_arguments[1]->is_error())
        return frame.compiled_arguments[1];

    auto expr1 = std::dynamic_pointer_cast<const instruction>(frame.compiled_arguments[0]);
    auto expr2 = std::dynamic_pointer_cast<const instruction>(frame.compiled_arguments[1]);
    assert(expr1);
    assert(expr2);

    auto new_expr = context.interpreter->cache_instruction_binary.get(
        operation,
        std::move(expr1),
        std::move(expr2),
        return_type);

    auto element = evaluate(context, std::move(new_expr));
    assert(element->actual_type == return_type);
    return element;
}