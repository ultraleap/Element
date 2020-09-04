#include "intrinsic_for.hpp"

//SELF
#include "object_model/error.hpp"
#include "object_model/intermediaries/for_wrapper.hpp"
#include "object_model/intermediaries/function_instance.hpp"

using namespace element;

intrinsic_for::intrinsic_for()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr intrinsic_for::compile(const compilation_context& context,
                                              const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *frame.function;
    assert(declarer.inputs.size() == 3);
    assert(frame.compiled_arguments.size() == 3);

    const auto initial = frame.compiled_arguments[0];
    const auto pred = std::dynamic_pointer_cast<const function_instance>(frame.compiled_arguments[1]);
    const auto body = std::dynamic_pointer_cast<const function_instance>(frame.compiled_arguments[2]);

    return for_wrapper::create_or_optimise(initial, pred, body, source_info, context);
}