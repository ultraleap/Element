#include "intrinsic_for.hpp"

//SELF
#include "object_model/error.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/declarations/struct_declaration.hpp"

using namespace element;

intrinsic_for::intrinsic_for()
    : intrinsic_function(type_id, nullptr)
{
}


/*
constant for loop if all of the objects (initial, predicate, body) are constant
    reimplement the for loop in evaluator

else it's a dynamic for loop

if to_expression fails on initial, error message, since it needs to be serializable
if predicate or body take inputs that are not serializable, error message, since they're almost
the same as a boundary function (except the return, we know it's the same type as initial)

if initial is an expression, create for instruction

else initial is an object, e.g. struct instance
    create for wrapper, so that when we index it, e.g. Vector2(0, 0).x, we create an
    indexing expression with the for loop as a child, or we need to make another for
    wrapper if the indexed thing is not a number (e.g. another struct instance)
 */

auto clone_instance_and_fill_with_indexing_expression(const compilation_context& context,
    const std::shared_ptr<const element_expression_for>& for_expression,
    const std::shared_ptr<const struct_instance>& instance,
    int& index) -> std::shared_ptr<struct_instance>
{
    auto clone = std::make_shared<struct_instance>(instance->declarer);

    for (const auto& [name, field] : instance->fields)
    {
        const auto* field_type = field->get_constraint();
        const auto* field_type_declarer = field_type->declarer;

        //if there's no declarer then we're probably dealing with a Num or Bool (IIRC)

        const auto field_as_expression = std::dynamic_pointer_cast<const element_expression>(field);

        if (field_as_expression)
        {
            assert(!field_type_declarer);
            auto thing = std::make_shared<element_expression_indexer>(for_expression, index, field_as_expression->actual_type);
            clone->fields.try_emplace(name, thing);
            index += 1;
            continue;
        }

        const auto field_as_instance = std::dynamic_pointer_cast<const struct_instance>(field);
        if (field_as_instance)
        {
            auto sub_clone = clone_instance_and_fill_with_indexing_expression(context, for_expression, field_as_instance, index);
            clone->fields.try_emplace(name, std::move(sub_clone));
            continue;
        }

        clone->fields.try_emplace(name, std::make_shared<const error>("???????????????????????????", ELEMENT_ERROR_UNKNOWN, source_information{}));
    }

    return clone;
};

object_const_shared_ptr create_or_optimise(const object_const_shared_ptr& initial_object,
    const std::shared_ptr<const function_instance>& predicate_function,
    const std::shared_ptr<const function_instance>& body_function,
    const source_information& source_info,
    const compilation_context& context)
{
    //todo: delete?
    /*auto initial_error = std::dynamic_pointer_cast<const error>(initial_object);
    if (initial_error)
        return initial_error;

    auto predicate_error = std::dynamic_pointer_cast<const error>(predicate_object);
    if (predicate_error)
        return predicate_error;

    auto body_error = std::dynamic_pointer_cast<const error>(body_object);
    if (body_error)
        return body_error;

    const auto initial_expression = std::dynamic_pointer_cast<const element_expression>(initial_object);
    if (!initial_expression)
    {
        return std::make_shared<const error>(
            "Initial element must be a value type, functions are not valid as the initial parameter",
            ELEMENT_ERROR_UNKNOWN, source_info);
    }
    */

    //try to run the loop at compile time, if the initial is constant
    const auto is_constant = initial_object->is_constant();

    //note: the predicate and the body could still return something which is not constant, so we need to check each time and try a runtime loop if so
    if (is_constant)
    {
        bool predicate_is_constant = true;

        const auto continue_loop = [&predicate_is_constant, &predicate_function, &context, &source_info](const std::vector<object_const_shared_ptr>& input) -> bool
        {
            const auto ret = predicate_function->call(context, input, source_info);
            if (!ret->is_constant())
            {
                predicate_is_constant = false;
                return false;
            }

            //todo: one day we'll use the fast RTTI instead of the language one
            const auto ret_as_constant = std::dynamic_pointer_cast<const element_expression_constant>(ret);
            assert(ret_as_constant);
            if (!ret_as_constant)
            {
                predicate_is_constant = false;
                return false;
            }

            return ret_as_constant->value() > 0;
        };

        const auto next_successor = [&initial_object, &body_function, &context, &source_info](const std::vector<object_const_shared_ptr>& input) -> object_const_shared_ptr
        {
            auto ret = body_function->call(context, input, source_info);
            if (!ret->is_constant())
                return nullptr;

            //todo: we could allow for a compile-time for loop to return a different type than it started with, but for now let's check
            if (!ret->matches_constraint(context, initial_object->get_constraint()))
                return nullptr;

            return ret;
        };

        std::vector<object_const_shared_ptr> arguments{ initial_object };
        auto& current_object = arguments[0];

        while (continue_loop(arguments))
        {
            current_object = next_successor(arguments);
            if (!current_object)
                break;
        }

        if (predicate_is_constant && current_object)
            return current_object;
    }

    //if you got this far, then something in the constant for loop case
    //was found to be non-constant which means we have a dynamic for loop

    const auto predicate_is_boundary = predicate_function->valid_at_boundary(context);
    const auto body_is_boundary = body_function->valid_at_boundary(context);
    if (!predicate_is_boundary)
        return std::make_shared<const error>("predicate is not a boundary function", ELEMENT_ERROR_UNKNOWN, source_info);

    if (!body_is_boundary)
        return std::make_shared<const error>("body is not a boundary function", ELEMENT_ERROR_UNKNOWN, source_info);

    const auto compile_function_instance = [](const compilation_context& context, const function_instance& function, const source_information& source_info) -> std::shared_ptr<const element_expression>
    {
        const auto generate_placeholder_inputs = [](const compilation_context& context, const function_declaration& declaration) -> std::optional<std::pair<std::vector<object_const_shared_ptr>, size_t>>
        {
            std::pair<std::vector<object_const_shared_ptr>, size_t> placeholder_inputs;
            int placeholder_index = context.total_boundary_size_at_index(context.boundaries.size() - 1);

            for (const auto& input : declaration.get_inputs())
            {
                auto placeholder = input.generate_placeholder(context, placeholder_index);
                if (!placeholder)
                    return {};

                placeholder_inputs.first.push_back(std::move(placeholder));
            }

            placeholder_inputs.second = placeholder_index;
            return placeholder_inputs;
        };

        auto placeholder_inputs = generate_placeholder_inputs(context, *function.declarer);
        assert(placeholder_inputs);
        if (!placeholder_inputs)
            return nullptr;

        context.boundaries.push_back({ placeholder_inputs.value().second });
        const auto compiled = function.call(context, std::move(placeholder_inputs.value().first), source_info);
        context.boundaries.pop_back();
        assert(compiled);

        if (!compiled)
            return nullptr;

        const auto err = std::dynamic_pointer_cast<const element::error>(compiled);
        if (err) {
            auto result = err->log_once(context.get_logger());
            return nullptr;
        }

        return compiled->to_expression();
    };

    auto predicate_compiled = compile_function_instance(context, *predicate_function, source_info);
    auto body_compiled = compile_function_instance(context, *body_function, source_info);

    if (!predicate_compiled)
        return std::make_shared<const error>("predicate failed to compile to an expression tree", ELEMENT_ERROR_UNKNOWN, source_info);

    if (!body_compiled)
        return std::make_shared<const error>("body failed to compile to an expression tree", ELEMENT_ERROR_UNKNOWN, source_info);

    auto initial_expression = std::dynamic_pointer_cast<const element_expression>(initial_object);
    //everything can be represented as an instruction, so make a for instruction
    if (initial_expression)
        return std::make_shared<element_expression_for>(std::move(initial_expression), std::move(predicate_compiled), std::move(body_compiled));

    initial_expression = initial_object->to_expression();
    if (!initial_expression)
        return std::make_shared<const error>("tried to create a runtime for but a non-serializable initial value was given", ELEMENT_ERROR_UNKNOWN, source_info);

    //  initial is an object, e.g. struct instance
    //    create for wrapper, so that when we later index it, e.g. Vector2(0, 0).x, we create an indexing expression with the for loop as a child

    /*
     * instead of a for wrapper we can create a struct instance where its fields are either indexing expressions with the for loop + index as a child, or another struct instance that is the same
     * i.e. from_expressions
     */

    const auto for_expression = std::make_shared<element_expression_for>(std::move(initial_expression), std::move(predicate_compiled), std::move(body_compiled));
    const auto initial_struct = std::dynamic_pointer_cast<const struct_instance>(initial_object);

    int index = 0;
    return clone_instance_and_fill_with_indexing_expression(context, for_expression, initial_struct, index);
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

    return create_or_optimise(initial, pred, body, source_info, context);
}