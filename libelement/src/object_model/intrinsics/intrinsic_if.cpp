#include "intrinsic_if.hpp"

//SELF
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/error.hpp"
#include "object_model/intermediaries/list_wrapper.hpp"

using namespace element;

intrinsic_if::intrinsic_if()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr intrinsic_if::compile(const compilation_context& context,
                                              const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *static_cast<const declaration*>(frame.function->declarer);
    assert(declarer.inputs.size() == 3);
    assert(frame.compiled_arguments.size() == 3);

    //todo: this flips the true/false branch so as not to modify the predicate, and we're not using the if expression, we probably want a different solution for niceties but this will work for now
    auto pred_expr = std::dynamic_pointer_cast<const instruction>(frame.compiled_arguments[0]);
    pred_expr->actual_type = type::num.get();
    return list_wrapper::create_or_optimise(std::move(pred_expr), { frame.compiled_arguments[2], frame.compiled_arguments[1] }, source_info);
}
