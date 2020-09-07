#include "etree/evaluator.hpp"

//SELF
#include <algorithm>
#include <cassert>
#include <cmath>

struct evaluator_ctx
{
    const element_value* inputs;
    const size_t inputs_count;
    element_evaluator_options options;
};

static element_result do_evaluate(evaluator_ctx& context, const expression_const_shared_ptr& expr, element_value* outputs, size_t outputs_count, size_t& outputs_written)
{
    if (const auto* ec = expr->as<element_expression_constant>()) {
        assert(outputs_count > outputs_written);
        outputs[outputs_written++] = ec->value();
        return ELEMENT_OK;
    } 

	if (const auto* ei = expr->as<element_expression_input>()) {
        if (context.inputs_count <= ei->index()
            || outputs_count <= outputs_written)
        {
            outputs_written = 0;
            return ELEMENT_ERROR_UNKNOWN;
        }

        assert(context.inputs_count > ei->index());
        assert(outputs_count > outputs_written);
        outputs[outputs_written++] = context.inputs[ei->index()];
        return ELEMENT_OK;
    } 

	if (const auto* es = expr->as<element_expression_structure>()) {
        auto deps = es->dependents();
        for (auto& dep : deps)
        {
            ELEMENT_OK_OR_RETURN(do_evaluate(context, dep, outputs, outputs_count, outputs_written));
        }
        return ELEMENT_OK;
    }
	
    if (const auto* eu = expr->as<element_expression_nullary>()) {
        assert(outputs_count > outputs_written);
        assert(eu->get_size() == 1);
        outputs[outputs_written++] = element_evaluate_nullary(eu->operation());
        return ELEMENT_OK;
    }

	if (const auto* eu = expr->as<element_expression_unary>()) {
        assert(outputs_count > outputs_written);
        assert(eu->input()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value a;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eu->input(), &a, 1, intermediate_written));
        outputs[outputs_written++] = element_evaluate_unary(eu->operation(), a);
        return ELEMENT_OK;
    } 

	if (const auto* eb = expr->as<element_expression_binary>()) {
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
    if (const auto* eb = expr->as<element_expression_if>()) {
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

    if (const auto* eb = expr->as<element_expression_for>()) {
        assert(outputs_count > outputs_written);
        assert(eb->initial()->get_size() == 1);
        assert(eb->condition()->get_size() == 1);
        assert(eb->body()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value initial;
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->initial(), &initial, 1, intermediate_written));
        intermediate_written = 0;
        auto for_result = element_evaluate_for(context, eb->initial(), eb->condition(), eb->body());
        assert(outputs_count >= outputs_written + for_result.size());
        assert(for_result.size() == eb->initial()->get_size());
        std::copy(for_result.begin(), for_result.begin() + for_result.size(), &outputs[outputs_written]);
        //std::memcpy(outputs + outputs_written, for_result.data(), for_result.size() * );
        outputs_written += for_result.size();
        return ELEMENT_OK;
    }

    if (const auto* eb = expr->as<element_expression_indexer>()) {
        assert(outputs_count > outputs_written);
        size_t intermediate_written = 0;
        size_t size = eb->for_expression->get_size();
        assert(eb->index < size);
        std::vector<element_value> for_result(size);
        ELEMENT_OK_OR_RETURN(do_evaluate(context, eb->for_expression, for_result.data(), size, intermediate_written));
        intermediate_written = 0;
        outputs[outputs_written++] = for_result[eb->index];
        return ELEMENT_OK;
    }

    if (const auto* sel = expr->as<element_expression_select>()) {
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
    expression_const_shared_ptr fn,
    const std::vector<element_value>& inputs,
    std::vector<element_value>& outputs,
    const element_evaluator_options opts)
{
    auto size = outputs.size();
    return element_evaluate(context, std::move(fn), inputs.data(), inputs.size(), outputs.data(), size, std::move(opts));
}

element_result element_evaluate(
    element_interpreter_ctx& context,
    expression_const_shared_ptr fn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t& outputs_count,
    element_evaluator_options opts)
{
    evaluator_ctx ectx = { inputs, inputs_count, std::move(opts) };
    size_t outputs_written = 0;
    const auto result = do_evaluate(ectx, fn, outputs, outputs_count, outputs_written);
    outputs_count = outputs_written;
    return result;
}

element_value element_evaluate_nullary(element_expression_nullary::op op)
{
    switch (op) {
    //num
    case element_expression_nullary::op::positive_infinity: return std::numeric_limits<float>::infinity();
    case element_expression_nullary::op::negative_infinity: return -std::numeric_limits<float>::infinity();
    case element_expression_nullary::op::nan:               return std::numeric_limits<float>::quiet_NaN();

    //boolean
    case element_expression_nullary::op::true_value:        return 1;
    case element_expression_nullary::op::false_value:       return 0;
    default: assert(false);                                 return static_cast<element_value>(std::nan(""));
    }
}

element_value element_evaluate_unary(element_expression_unary::op op, element_value a)
{
    switch (op) {
    //num
    case element_expression_unary::op::abs:   return std::fabs(a);
    case element_expression_unary::op::acos:  return std::acos(a);
    case element_expression_unary::op::asin:  return std::asin(a);
    case element_expression_unary::op::atan:  return std::atan(a);
    case element_expression_unary::op::ceil:  return std::ceil(a);
    case element_expression_unary::op::cos:   return std::cos(a);
    case element_expression_unary::op::floor: return std::floor(a);
    case element_expression_unary::op::ln:    return std::log(a);
    case element_expression_unary::op::sin:   return std::sin(a);
    case element_expression_unary::op::tan:   return std::tan(a);

    //boolean
    case element_expression_unary::op::not_ :  return !a;
    default: assert(false);                   return static_cast<element_value>(std::nan(""));
    }
}

element_value element_evaluate_binary(element_expression_binary::op op, element_value a, element_value b)
{
    switch (op) {
        //num
    case element_expression_binary::op::add:   return a + b;
    case element_expression_binary::op::atan2: return std::atan2(a, b);
    case element_expression_binary::op::div:   return a / b;
    case element_expression_binary::op::log:   return b ? std::log10(a) / std::log10(b) : std::nanf(""); // TODO: optimised pseudo-ops for log10 et al?
    case element_expression_binary::op::max:   return (std::max)(a, b);
    case element_expression_binary::op::min:   return (std::min)(a, b);
    case element_expression_binary::op::mul:   return a * b;
    case element_expression_binary::op::pow:   return std::pow(a, b);
    case element_expression_binary::op::rem:   return std::fmod(a, b);
    case element_expression_binary::op::sub:   return a - b;

    //boolean
    case element_expression_binary::op::and_ :  return a && b;
    case element_expression_binary::op:: or_ :  return a || b;

    //comparison
    case element_expression_binary::op::eq :   return a == b;
    case element_expression_binary::op::neq:   return a != b;
    case element_expression_binary::op::lt:    return a < b;
    case element_expression_binary::op::leq:   return a <= b;
    case element_expression_binary::op::gt:    return a > b;
    case element_expression_binary::op::geq:   return a >= b;
    default: assert(false);                    return static_cast<element_value>(std::nan(""));
    }
}

//TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
element_value element_evaluate_if(element_value predicate, element_value if_true, element_value if_false)
{
    //Element treats negative numbers and 0 as false
    return predicate > 0 ? if_true : if_false;
}

std::vector<element_value> element_evaluate_for(evaluator_ctx& context, const expression_const_shared_ptr& initial, const expression_const_shared_ptr& condition, const expression_const_shared_ptr& body)
{
    size_t intermediate_written = 0;

    const auto current_value_offset = context.inputs_count;

    const auto value_size = initial->get_size();
    std::vector<element_value> inputs;
    inputs.resize(context.inputs_count + value_size);

    std::copy(context.inputs, context.inputs + context.inputs_count, inputs.data());

    evaluator_ctx ectx = { inputs.data(), inputs.size(), {} };

    auto result = do_evaluate(context, initial, inputs.data() + current_value_offset, value_size, intermediate_written);
    if (result != ELEMENT_OK)
        throw;

    intermediate_written = 0;

    element_value predicate_value;
    result = do_evaluate(ectx, condition, &predicate_value, 1, intermediate_written);
    intermediate_written = 0;

    while (predicate_value > 0) //predicate returned true
    {
        result = do_evaluate(ectx, body, inputs.data() + current_value_offset, value_size, intermediate_written);
        if (result != ELEMENT_OK)
            throw;

        if (intermediate_written != value_size)
            throw;

        intermediate_written = 0;
        result = do_evaluate(ectx, condition, &predicate_value, 1, intermediate_written);
        if (result != ELEMENT_OK)
            throw;

        if (intermediate_written != 1)
            throw;

        intermediate_written = 0;
    }

    if (result != ELEMENT_OK)
        throw;

    return std::vector<element_value>(inputs.begin() + current_value_offset, inputs.end());
}

expression_const_shared_ptr element_evaluate_select(element_value selector, std::vector<expression_const_shared_ptr> options)
{
    assert(!options.empty());
    const auto clamped_index = std::clamp(static_cast<int>(selector), 0, static_cast<int>(options.size() - 1));
    return options[clamped_index];
}