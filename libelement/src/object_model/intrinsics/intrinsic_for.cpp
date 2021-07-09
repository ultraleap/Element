#include "intrinsic.hpp"
#include "intrinsic_for.hpp"

//SELF
#include "object_model/error.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/declarations/struct_declaration.hpp"
#include "object_model/intrinsics/intrinsic_function.hpp"

using namespace element;

object_const_shared_ptr compile_time_for(const object_const_shared_ptr& initial_object,
    const std::shared_ptr<const function_instance>& predicate_function,
    const std::shared_ptr<const function_instance>& body_function,
    const source_information& source_info,
    const compilation_context& context)
{
    const auto is_constant = initial_object->is_constant();
    if (!is_constant)
        return nullptr;

    //note: the predicate and the body could still return something which is not constant, so we need to check constantly
    bool predicate_evaluated_to_constant = true;

    const auto continue_loop = [&predicate_evaluated_to_constant, &predicate_function, &context, &source_info](const std::vector<object_const_shared_ptr>& input) -> bool {
        const auto ret = predicate_function->call(context, input, source_info);

        if (ret->is_error()) {
            ret->log_any_error(context.get_logger());
            return false;
        }

        //todo: one day we'll use the fast RTTI instead of the language one
        //the return value is going to be a bool (of some kind), and bools are expressions
        const auto ret_as_expression = std::dynamic_pointer_cast<const instruction>(ret);
        if (!ret_as_expression || !ret_as_expression->is_constant()) {
            predicate_evaluated_to_constant = false;
            return false;
        }

        const auto ret_evaluated = evaluate(context, ret_as_expression);
        assert(ret_evaluated);
        const auto ret_as_constant = std::dynamic_pointer_cast<const element::instruction_constant>(ret_evaluated);
        if (!ret_as_constant) {
            predicate_evaluated_to_constant = false;
            return false;
        }

        return to_bool(ret_as_constant->value());
    };

    const auto next_successor = [&initial_object, &body_function, &context, &source_info](const std::vector<object_const_shared_ptr>& input) -> object_const_shared_ptr {
        auto ret = body_function->call(context, input, source_info);
        if (!ret->is_constant())
            return nullptr;

        if (ret->is_error()) {
            ret->log_any_error(context.get_logger());
            return ret;
        }

        //todo: we could allow for a compile-time for loop to return a different type than it started with, but for now let's check
        if (!ret->matches_constraint(context, initial_object->get_constraint()))
            return nullptr;

        return ret;
    };

    std::vector<object_const_shared_ptr> arguments{ initial_object };
    auto& current_object = arguments[0];

    //todo: in order to detect an infinite loop we need to know if the predicates result is dependent on its input, i.e. it is actually using it to alter the calculation in a meaningful way
    //  not sure how to do it, but the loop iteration limit will catch the infinite loop situation anyway

    //todo: make this user configurable
    constexpr auto max_loop_iterations = 10'000;
    auto current_loop_iteration = 0;
    while (continue_loop(arguments)) {
        if (current_loop_iteration > max_loop_iterations)
            return std::make_shared<const error>(
                fmt::format("Compile time loop didn't finish after max iteration count of {}", max_loop_iterations),
                ELEMENT_ERROR_INFINITE_LOOP,
                source_info,
                context.get_logger());

        current_object = next_successor(arguments);
        if (!current_object)
            return current_object;
        ++current_loop_iteration;
    }

    if (!predicate_evaluated_to_constant)
        return nullptr;

    return current_object;
}

object_const_shared_ptr runtime_for(const object_const_shared_ptr& initial_object,
    const std::shared_ptr<const function_instance>& predicate_function,
    const std::shared_ptr<const function_instance>& body_function,
    const source_information& source_info,
    const compilation_context& context)
{
    //ensure that these are boundary functions as we'll need to compile them like any other boundary function
    const auto predicate_is_boundary = predicate_function->valid_at_boundary(context);
    if (!predicate_is_boundary)
        return std::make_shared<const error>(
            "predicate is not a boundary function",
            ELEMENT_ERROR_UNKNOWN,
            predicate_function->source_info,
            context.get_logger());

    const auto body_is_boundary = body_function->valid_at_boundary(context);
    if (!body_is_boundary)
        return std::make_shared<const error>(
            "body is not a boundary function",
            ELEMENT_ERROR_UNKNOWN,
            body_function->source_info,
            context.get_logger());

    //compile our functions to instruction trees, with their own placeholder input instructions
    auto predicate_compiled = compile_placeholder_expression(context, *predicate_function, predicate_function->get_inputs(), source_info);
    auto body_compiled = compile_placeholder_expression(context, *body_function, body_function->get_inputs(), source_info);

    if (predicate_compiled->is_error())
        return predicate_compiled;

    if (!predicate_compiled)
        return std::make_shared<const error>("predicate failed to compile", ELEMENT_ERROR_UNKNOWN, source_info, context.get_logger());

    if (body_compiled->is_error())
        return body_compiled;

    if (!body_compiled)
        return std::make_shared<const error>("body failed to compile", ELEMENT_ERROR_UNKNOWN, source_info, context.get_logger());

    auto predicate_expression = predicate_compiled->to_instruction(*context.interpreter);
    if (!predicate_expression)
        return std::make_shared<const error>("predicate failed to compile to an instruction tree", ELEMENT_ERROR_UNKNOWN, predicate_function->source_info, context.get_logger());

    auto body_expression = body_compiled->to_instruction(*context.interpreter);
    if (!body_expression)
        return std::make_shared<const error>("body failed to compile to an instruction tree", ELEMENT_ERROR_UNKNOWN, body_function->source_info, context.get_logger());

    //everything is an instruction, so make a for instruction. note: initial_object is either Num or Bool.
    auto initial_expression = std::dynamic_pointer_cast<const instruction>(initial_object);
    if (initial_expression)
        return context.interpreter->cache_instruction_for.get(std::move(initial_expression), std::move(predicate_expression), std::move(body_expression), context.boundaries.back().inputs);

    //if it wasn't an instruction, let's convert it in to one.
    //if we can't then this for-loop can't be done at runtime, since it can't be represented in the instruction tree
    initial_expression = initial_object->to_instruction(*context.interpreter);
    if (!initial_expression)
        return std::make_shared<const error>("tried to create a runtime for but a non-serializable initial value was given", ELEMENT_ERROR_UNKNOWN, source_info, context.get_logger());

    //initial_object should be a struct, so the output of the for loop is going to be the same type of struct, except all the fields (flattened struct) are instructions referring to the for loop
    const auto for_expression = context.interpreter->cache_instruction_for.get(std::move(initial_expression), std::move(predicate_expression), std::move(body_expression), context.boundaries.back().inputs);
    const auto initial_struct = std::dynamic_pointer_cast<const struct_instance>(initial_object);

    auto indexing_expression_filler = [&for_expression, &context](const std::string&,
                                          const std::shared_ptr<const instruction>& field,
                                          int index) -> std::shared_ptr<const instruction> {
        return context.interpreter->cache_instruction_indexer.get(for_expression, index, field->actual_type);
    };

    return initial_struct->clone_and_fill_with_expressions(context, std::move(indexing_expression_filler));
}

intrinsic_for::intrinsic_for()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr intrinsic_for::compile(const compilation_context& context,
    const source_information& source_info) const
{
    const auto& frame = context.calls.frames.back();
    const auto& declarer = *static_cast<const declaration*>(frame.function->declarer);
    assert(declarer.inputs.size() == 3);
    assert(frame.compiled_arguments.size() == 3);

    auto initial = frame.compiled_arguments[0];
    const auto predicate = std::dynamic_pointer_cast<const function_instance>(frame.compiled_arguments[1]);
    const auto body = std::dynamic_pointer_cast<const function_instance>(frame.compiled_arguments[2]);

    assert(initial);
    assert(predicate);
    assert(body);

    if (initial->is_error())
        return initial;

    auto compile_time_result = compile_time_for(initial, predicate, body, source_info, context);
    if (compile_time_result)
        return compile_time_result;

    return runtime_for(initial, predicate, body, source_info, context);
}