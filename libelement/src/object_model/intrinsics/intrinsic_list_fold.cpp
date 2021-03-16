#include "intrinsic.hpp"
#include "intrinsic_list_fold.hpp"

//SELF
#include "object_model/error.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/declarations/struct_declaration.hpp"

using namespace element;

intrinsic_list_fold::intrinsic_list_fold()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr compile_time_fold(
    const compilation_context& context,
    const std::shared_ptr<const struct_instance>& list,
    const object_const_shared_ptr& initial,
    const std::shared_ptr<const function_instance>& accumulator_function,
    const source_information& source_info)
{
    const auto is_constant = list->is_constant();
    if (!is_constant)
        return nullptr;

    const auto list_count = list->index(context, identifier::list_count_identifier, source_info)->compile(context, source_info);
    if (!list_count->is_constant())
        return nullptr;

    const auto list_count_constant = std::dynamic_pointer_cast<const element::instruction_constant>(list_count);

    std::vector<object_const_shared_ptr> indexer_arguments;
    indexer_arguments.resize(1);

    auto aggregate = initial;
    const auto list_at = list->index(context, identifier::list_at_identifier, source_info);
    for (int i = 0; i < list_count_constant->value(); ++i)
    {
        indexer_arguments[0] = std::make_shared<const instruction_constant>(static_cast<element_value>(i));
        auto list_element = list_at->call(context, indexer_arguments, source_info);
        if (!list_element->is_constant())
            return nullptr;

        //note: the order must be maintained across compilers to ensure the same results for non-commutative operations
        aggregate = accumulator_function->call(context, { std::move(aggregate), std::move(list_element) }, source_info);
    }

    return aggregate;
}

object_const_shared_ptr runtime_fold(
    const compilation_context& context,
    const std::shared_ptr<const struct_instance>& list,
    const object_const_shared_ptr& initial,
    const std::shared_ptr<const function_instance>& accumulator_function,
    const source_information& source_info)
{
    const auto accumulator_is_boundary = accumulator_function->valid_at_boundary(context);
    if (!accumulator_is_boundary)
        return std::make_shared<const error>(
            "accumulator is not a boundary function",
            ELEMENT_ERROR_UNKNOWN,
            accumulator_function->source_info,
            context.get_logger());

    auto accumulator_compiled = compile_placeholder_expression(context, *accumulator_function, accumulator_function->get_inputs(), source_info);
    if (accumulator_compiled->is_error())
        return accumulator_compiled;

    if (!accumulator_compiled)
        return std::make_shared<const error>("accumulator failed to compile", ELEMENT_ERROR_UNKNOWN, source_info, context.get_logger());

    const auto accumulator_expression = accumulator_compiled->to_instruction();
    if (!accumulator_expression)
        return std::make_shared<const error>(
            "accumulator failed to compile to an instruction tree",
            ELEMENT_ERROR_UNKNOWN,
            accumulator_function->source_info,
            context.get_logger());

    const auto* listfold = context.get_compiler_scope()->find(identifier{ "@list_fold" }, context.interpreter->caches, false);
    if (!listfold)
        return std::make_shared<const error>("failed to find @list_fold", ELEMENT_ERROR_UNKNOWN, source_info, context.get_logger());

    std::vector<object_const_shared_ptr> list_fold_args{ list, initial, accumulator_function };
    return listfold->call(context, std::move(list_fold_args), source_info);
}

object_const_shared_ptr intrinsic_list_fold::compile(
    const compilation_context& context,
    const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *static_cast<const declaration*>(frame.function->declarer);
    assert(declarer.inputs.size() == 3);
    assert(frame.compiled_arguments.size() == 3);

    const auto& list = frame.compiled_arguments[0];
    const auto& initial = frame.compiled_arguments[1];
    const auto& accumulator = frame.compiled_arguments[2];

    const auto list_struct = std::dynamic_pointer_cast<const struct_instance>(list);
    if (!list_struct)
        return std::make_shared<const error>(
            "first argument must be a list struct instance",
            ELEMENT_ERROR_UNKNOWN,
            source_info,
            context.get_logger());

    const auto accumulator_instance = std::dynamic_pointer_cast<const function_instance>(accumulator);
    if (!accumulator_instance)
        return std::make_shared<const error>(
            "first argument must be a binary function instance",
            ELEMENT_ERROR_UNKNOWN,
            source_info,
            context.get_logger());

    auto compile_time_result = compile_time_fold(context, list_struct, initial, accumulator_instance, source_info);
    if (compile_time_result)
        return compile_time_result;

    return runtime_fold(context, list_struct, initial, accumulator_instance, source_info);
}