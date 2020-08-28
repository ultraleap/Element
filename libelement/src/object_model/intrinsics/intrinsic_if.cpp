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

    //if the predicate is a constant then just return the correct branch
    const auto* predicate_as_constant = dynamic_cast<const element_expression_constant*>(frame.compiled_arguments[0].get());
    if (predicate_as_constant)
        return predicate_as_constant->value() > 0 ? frame.compiled_arguments[1] : frame.compiled_arguments[2];

    //the predicate is dynamic, and the branches are expressions (constant or dynamic), so create a tree to evaluate the correct branch later when we can determine the predicate from user input
    auto pred_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
    auto true_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[1]);
    auto false_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[2]);
    const bool all_expressions = pred_expr && true_expr && false_expr;
    if (all_expressions)
    {
        return std::make_unique<element_expression_if>(
            pred_expr,
            true_expr,
            false_expr);
    }

    //the predicate is dynamic and the branches aren't expressions. this is where we need a wrapper like ListElement to delay which branch we pick until we know more, since we can't have intermediaries in our expression tree
    return std::make_shared<error>("predicate for Bool.if must be a compile-time constant as the branches are not expressions", ELEMENT_ERROR_UNKNOWN, source_info);
}