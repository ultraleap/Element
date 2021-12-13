#include "intrinsic_if.hpp"

//SELF
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/error.hpp"
#include "object_model/intermediaries/list_wrapper.hpp"
#include "object_model/declarations//function_declaration.hpp"

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

    auto pred_expr = std::dynamic_pointer_cast<const instruction>(frame.compiled_arguments[0]);
    if (!pred_expr) {
        return std::make_shared<const element::error>(
            fmt::format("First argument to intrinsic 'if' was '{}' which is invalid.",
                frame.compiled_arguments[0]->to_string()),
            ELEMENT_ERROR_UNKNOWN, source_info);
    }

    auto& true_decl = *context.get_global_scope()->find(identifier("True"), context.interpreter->cache_scope_find, false);
    auto& false_decl = *context.get_global_scope()->find(identifier("False"), context.interpreter->cache_scope_find, false);

    const auto true_expr = get_intrinsic(context.interpreter, true_decl)->compile(context, source_info);
    const auto false_expr = get_intrinsic(context.interpreter, false_decl)->compile(context, source_info);

    auto wrapped_pre_expr = context.interpreter->cache_instruction_if.get(
        std::move(pred_expr),
        std::dynamic_pointer_cast<const instruction>(true_expr),
        std::dynamic_pointer_cast<const instruction>(false_expr));

    auto maybe_constant_wrapped_pre_expr = evaluate(context, std::move(wrapped_pre_expr));
    if (maybe_constant_wrapped_pre_expr->actual_type != type::boolean.get())
        throw;

    //todo: this flips the true/false branch so as not to modify the predicate, and we're not using the if expression, we probably want a different solution for niceties but this will work for now
    return list_wrapper::create_or_optimise(*context.interpreter, std::move(maybe_constant_wrapped_pre_expr), { frame.compiled_arguments[2], frame.compiled_arguments[1] }, source_info);
}
