#include "intrinsic_for.hpp"

//SELF
#include "object_model/error.hpp"

using namespace element;

intrinsic_for::intrinsic_for()
    : intrinsic_function(type_id, nullptr)
{
}

//Calls body repeatedly until condition is not met
//WARNING: Usage of this function breaks halting guarantees
//Body is a Unary function supplied with the output from the previous body starting with initial
//Thus the types of initialand both the parameterand return of Unary must have compatible interfaces
//List.fold is recommended as a constant - time alternative when iteration count is known
//intrinsic function for (initial, condition : Predicate, body : Unary)

object_const_shared_ptr intrinsic_for::compile(const compilation_context& context,
                                              const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *frame.function;
    assert(declarer.inputs.size() == 3);
    assert(frame.compiled_arguments.size() == 3);

    auto initial_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[0]);
    auto pred_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[1]);
    auto body_expr = std::dynamic_pointer_cast<const element_expression>(frame.compiled_arguments[2]);

    return std::make_unique<element_expression_for>(
        initial_expr,
        pred_expr,
        body_expr);
}