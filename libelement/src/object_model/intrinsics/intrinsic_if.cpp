#include "intrinsic_if.hpp"

//SELF
#include "object_model/declarations/declaration.hpp"
#include "object_model/error.hpp"

using namespace element;

intrinsic_if::intrinsic_if()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr intrinsic_if::compile(const compilation_context& context,
                                                    const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *frame.function;
    assert(declarer.inputs.size() == 3);
    assert(frame.compiled_arguments.size() == 3);

    const auto intrinsic = get_intrinsic(context.interpreter, declarer);
    assert(intrinsic);
    assert(intrinsic == this);

    auto pred_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
    assert(pred_expr);

    //todo: hack. we can only do if-expressions if the predicate is a constant. the difficulty is in the branches returning a non-expression type

    std::vector<element_value> outputs = { 0 };
    const auto result = element_evaluate(*context.interpreter, pred_expr, {}, outputs, {});
    if (result != ELEMENT_OK)
        return std::make_shared<error>("predicate for Bool.if must be a compile-time constant", ELEMENT_ERROR_UNKNOWN, source_info); //todo

    return outputs[0] > 0 ? frame.compiled_arguments[1] : frame.compiled_arguments[2];

    //TODO: Remove zombie code
    auto true_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[1]);
    auto false_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[2]);
    assert(true_expr);
    assert(false_expr);

    auto ret = std::make_unique<element_expression_if>(
        pred_expr,
        true_expr,
        false_expr);

    return ret;
}