#include "instruction_tree/evaluator.hpp"

//SELF
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace element;

struct evaluator_ctx
{
    struct boundary
    {
        const element_value* inputs;
        const size_t inputs_count;
    };
    std::vector<boundary> boundaries;
    element_evaluator_options options;
};

static element_result do_evaluate(evaluator_ctx& context, const element::instruction_const_shared_ptr& expr, element_value* outputs, size_t outputs_count, size_t& outputs_written)
{
    if (const auto* ec = expr->as<element::instruction_constant>())
    {
        assert(outputs_count > outputs_written);
        outputs[outputs_written++] = ec->value();
        return ELEMENT_OK;
    }

    if (const auto* ei = expr->as<element::instruction_input>())
    {
        if (context.boundaries.size() <= ei->scope()
            || context.boundaries[ei->scope()].inputs_count <= ei->index()
            || outputs_count <= outputs_written)
        {
            //occurs during constant folding to check if it can be evaluated
            outputs_written = 0;
            return ELEMENT_ERROR_UNKNOWN;
        }

        assert(outputs_count > outputs_written);
        outputs[outputs_written++] = context.boundaries[ei->scope()].inputs[ei->index()];
        return ELEMENT_OK;
    }

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
    {
        auto deps = es->dependents();
        for (auto& dep : deps)
        {
            ELEMENT_OK_OR_RETURN(do_evaluate(context, dep, outputs, outputs_count, outputs_written));
        }
        return ELEMENT_OK;
    }

    if (const auto* eu = expr->as<element::instruction_nullary>())
    {
        assert(outputs_count > outputs_written);
        assert(eu->get_size() == 1);
        outputs[outputs_written++] = element_evaluate_nullary(eu->operation());
        return ELEMENT_OK;
    }

    if (const auto* eu = expr->as<element::instruction_unary>())
    {
        assert(outputs_count > outputs_written);
        assert(eu->input()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value a;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eu->input(), &a, 1, intermediate_written));
        outputs[outputs_written++] = element_evaluate_unary(eu->operation(), a);
        return ELEMENT_OK;
    }

    if (const auto* eb = expr->as<element::instruction_binary>())
    {
        assert(outputs_count > outputs_written);
        assert(eb->input1()->get_size() == 1);
        assert(eb->input2()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value a, b;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->input1(), &a, 1, intermediate_written));
        intermediate_written = 0;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->input2(), &b, 1, intermediate_written));
        outputs[outputs_written++] = element_evaluate_binary(eb->operation(), a, b);
        return ELEMENT_OK;
    }

    //TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
    if (const auto* eb = expr->as<element::instruction_if>())
    {
        assert(outputs_count > outputs_written);
        assert(eb->predicate()->get_size() == 1);
        assert(eb->if_true()->get_size() == 1);
        assert(eb->if_false()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value predicate, if_true, if_false;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->predicate(), &predicate, 1, intermediate_written));
        intermediate_written = 0;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->if_true(), &if_true, 1, intermediate_written));
        intermediate_written = 0;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->if_false(), &if_false, 1, intermediate_written));
        outputs[outputs_written++] = element_evaluate_if(predicate, if_true, if_false);
        return ELEMENT_OK;
    }

    if (const auto* eb = expr->as<element::instruction_for>())
    {
        assert(outputs_count > outputs_written);
        assert(eb->condition()->get_size() == 1);
        assert(eb->initial()->get_size() >= 1);
        assert(eb->body()->get_size() >= 1);
        assert(eb->body()->get_size() == eb->initial()->get_size());
        size_t intermediate_written = 0;
        std::vector<element_value> initial;
        initial.resize(eb->initial()->get_size());
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->initial(), initial.data(), eb->initial()->get_size(), intermediate_written));
        intermediate_written = 0;
        auto for_result = element_evaluate_for(context, eb->initial(), eb->condition(), eb->body());
        assert(outputs_count >= outputs_written + for_result.size());
        assert(for_result.size() == eb->initial()->get_size());
        std::copy(for_result.begin(), for_result.begin() + for_result.size(), &outputs[outputs_written]);
        //std::memcpy(outputs + outputs_written, for_result.data(), for_result.size() * );
        outputs_written += for_result.size();
        return ELEMENT_OK;
    }

    if (const auto* eb = expr->as<element::instruction_indexer>())
    {
        assert(outputs_count > outputs_written);
        size_t intermediate_written = 0;
        size_t size = eb->for_instruction->get_size();
        assert(eb->index < size);
        std::vector<element_value> for_result(size);
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->for_instruction, for_result.data(), size, intermediate_written));
        intermediate_written = 0;
        outputs[outputs_written++] = for_result[eb->index];
        return ELEMENT_OK;
    }

    if (const auto* sel = expr->as<element::instruction_select>())
    {
        assert(outputs_count > outputs_written);
        size_t intermediate_written = 0;
        element_value selector;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, sel->selector, &selector, 1, intermediate_written));
        intermediate_written = 0;
        const auto selected_option = element_evaluate_select(selector, sel->options);
        ELEMENT_OK_OR_RETURN(do_evaluate(context, selected_option, outputs, outputs_count, outputs_written));
        return ELEMENT_OK;
    }

    return ELEMENT_ERROR_NO_IMPL;
}

element_result element_evaluate(
    element_interpreter_ctx& context,
    const element::instruction_const_shared_ptr& fn,
    const std::vector<element_value>& inputs,
    std::vector<element_value>& outputs,
    const element_evaluator_options opts)
{
    auto size = outputs.size();
    auto result = element_evaluate(context, fn, inputs.data(), inputs.size(), outputs.data(), size, opts);
    outputs.resize(size);
    return result;
}

element_result element_evaluate(
    element_interpreter_ctx& context,
    const element::instruction_const_shared_ptr& fn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t& outputs_count,
    element_evaluator_options opts)
{
    evaluator_ctx ectx = { {}, opts };
    ectx.boundaries.push_back({ inputs, inputs_count });

    size_t outputs_written = 0;
    const auto result = do_evaluate(ectx, fn, outputs, outputs_count, outputs_written);
    outputs_count = outputs_written;
    return result;
}

element_value element_evaluate_nullary(element::instruction_nullary::op op)
{
    switch (op)
    {
    //num
    case element::instruction_nullary::op::positive_infinity:
        return std::numeric_limits<float>::infinity();
    case element::instruction_nullary::op::negative_infinity:
        return -std::numeric_limits<float>::infinity();
    case element::instruction_nullary::op::nan:
        return std::numeric_limits<float>::quiet_NaN();

    //boolean
    case element::instruction_nullary::op::true_value:
        return 1;
    case element::instruction_nullary::op::false_value:
        return 0;
    default:
        assert(false);
        return static_cast<element_value>(std::nan(""));
    }
}

element_value element_evaluate_unary(element::instruction_unary::op op, element_value a)
{
    switch (op)
    {
    //num
    case element::instruction_unary::op::abs:
        return std::fabs(a);
    case element::instruction_unary::op::acos:
        return std::acos(a);
    case element::instruction_unary::op::asin:
        return std::asin(a);
    case element::instruction_unary::op::atan:
        return std::atan(a);
    case element::instruction_unary::op::ceil:
        return std::ceil(a);
    case element::instruction_unary::op::cos:
        return std::cos(a);
    case element::instruction_unary::op::floor:
        return std::floor(a);
    case element::instruction_unary::op::ln:
        return std::log(a);
    case element::instruction_unary::op::sin:
        return std::sin(a);
    case element::instruction_unary::op::tan:
        return std::tan(a);

    //boolean
    case element::instruction_unary::op::not_:
        return !a;
    default:
        assert(false);
        return static_cast<element_value>(std::nan(""));
    }
}

element_value element_evaluate_binary(element::instruction_binary::op op, element_value a, element_value b)
{
    switch (op)
    {
        //num
    case element::instruction_binary::op::add:
        return a + b;
    case element::instruction_binary::op::atan2:
        return std::atan2(a, b);
    case element::instruction_binary::op::div:
        return a / b;
    case element::instruction_binary::op::log:
        return b ? std::log10(a) / std::log10(b) : std::nanf(""); // TODO: optimised pseudo-ops for log10 et al?
    case element::instruction_binary::op::max:
        return (std::max)(a, b);
    case element::instruction_binary::op::min:
        return (std::min)(a, b);
    case element::instruction_binary::op::mul:
        return a * b;
    case element::instruction_binary::op::pow:
        return std::pow(a, b);
    case element::instruction_binary::op::rem:
        return std::fmod(a, b);
    case element::instruction_binary::op::sub:
        return a - b;

    //boolean
    case element::instruction_binary::op::and_:
        return to_bool(a) && to_bool(b);
    case element::instruction_binary::op::or_:
        return to_bool(a) || to_bool(b);

    //comparison
    case element::instruction_binary::op::eq:
        return a == b;
    case element::instruction_binary::op::neq:
        return a != b;
    case element::instruction_binary::op::lt:
        return a < b;
    case element::instruction_binary::op::leq:
        return a <= b;
    case element::instruction_binary::op::gt:
        return a > b;
    case element::instruction_binary::op::geq:
        return a >= b;
    default:
        assert(false);
        return static_cast<element_value>(std::nan(""));
    }
}

//TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
element_value element_evaluate_if(element_value predicate, element_value if_true, element_value if_false)
{
    return to_bool(predicate) ? if_true : if_false;
}

std::vector<element_value> element_evaluate_for(evaluator_ctx& context, const element::instruction_const_shared_ptr& initial, const element::instruction_const_shared_ptr& condition, const element::instruction_const_shared_ptr& body)
{
    size_t intermediate_written = 0;

    const auto value_size = initial->get_size();
    std::vector<element_value> inputs;
    inputs.resize(value_size);
    context.boundaries.push_back({ inputs.data(), value_size });

    auto result = do_evaluate(context, initial, inputs.data(), value_size, intermediate_written);
    if (result != ELEMENT_OK)
        throw;

    intermediate_written = 0;

    element_value predicate_value;
    result = do_evaluate(context, condition, &predicate_value, 1, intermediate_written);
    intermediate_written = 0;

    if (result != ELEMENT_OK)
        throw;

    // Create a buffer for outputs, so that we do not modify 'inputs' during the evaluation
    // of the body.
    std::vector<element_value> body_output_buffer(value_size);

    while (to_bool(predicate_value)) //predicate returned true
    {
        result = do_evaluate(context, body, body_output_buffer.data(), value_size, intermediate_written);

        std::swap(body_output_buffer, inputs);
        context.boundaries.back().inputs = inputs.data();

        if (result != ELEMENT_OK)
            throw;

        if (intermediate_written != value_size)
            throw;

        intermediate_written = 0;
        result = do_evaluate(context, condition, &predicate_value, 1, intermediate_written);
        if (result != ELEMENT_OK)
            throw;

        if (intermediate_written != 1)
            throw;

        intermediate_written = 0;
    }

    if (result != ELEMENT_OK)
        throw;

    context.boundaries.pop_back();
    return inputs;
}

element::instruction_const_shared_ptr element_evaluate_select(element_value selector, std::vector<element::instruction_const_shared_ptr> options)
{
    assert(!options.empty());
    const auto clamped_index = std::clamp(static_cast<int>(selector), 0, static_cast<int>(options.size() - 1));
    return options[clamped_index];
}