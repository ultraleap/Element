#include "etree/evaluator.hpp"
#include <algorithm>
#include <cassert>

struct evaluator_ctx
{
    const element_value* inputs;
    const size_t inputs_count;
    element_evaluator_options options;
};


static element_result do_evaluate(evaluator_ctx& ctx, const expression_const_shared_ptr& expr, element_value* outputs, size_t outputs_count, size_t& outputs_written)
{
    if (auto ec = expr->as<element_constant>()) {
        assert(outputs_count > outputs_written);
        outputs[outputs_written++] = ec->value();
        return ELEMENT_OK;
    } else if (auto ei = expr->as<element_input>()) {
        assert(ctx.inputs_count > ei->index());
        assert(outputs_count > outputs_written);
        outputs[outputs_written++] = ctx.inputs[ei->index()];
        return ELEMENT_OK;
    } else if (auto es = expr->as<element_structure>()) {
        auto deps = es->dependents();
        for (size_t i = 0; i < deps.size(); ++i) {
            ELEMENT_OK_OR_RETURN(do_evaluate(ctx, deps[i], outputs, outputs_count, outputs_written));
        }
        return ELEMENT_OK;
    } else if (auto eu = expr->as<element_unary>()) {
        assert(outputs_count > outputs_written);
        assert(eu->input()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value a;
        ELEMENT_OK_OR_RETURN(do_evaluate(ctx, eu->input(), &a, 1, intermediate_written));
        outputs[outputs_written++] = element_evaluate_unary(eu->operation(), a);
        return ELEMENT_OK;
    } else if (auto eb = expr->as<element_binary>()) {
        assert(outputs_count > outputs_written);
        assert(eb->input1()->get_size() == 1);
        assert(eb->input2()->get_size() == 1);
        size_t intermediate_written = 0;
        element_value a, b;
        ELEMENT_OK_OR_RETURN(do_evaluate(ctx, eb->input1(), &a, 1, intermediate_written));
        intermediate_written = 0;
        ELEMENT_OK_OR_RETURN(do_evaluate(ctx, eb->input2(), &b, 1, intermediate_written));
        outputs[outputs_written++] = element_evaluate_binary(eb->operation(), a, b);
        return ELEMENT_OK;
    } else {
        return ELEMENT_ERROR_NO_IMPL;
    }
}


element_result element_evaluate(
    element_interpreter_ctx& ctx,
    expression_const_shared_ptr fn,
    const std::vector<element_value>& inputs,
    std::vector<element_value>& outputs,
    const element_evaluator_options opts)
{
    return element_evaluate(ctx, std::move(fn), inputs.data(), inputs.size(), outputs.data(), outputs.size(), std::move(opts));
}

element_result element_evaluate(
    element_interpreter_ctx& ctx,
    expression_const_shared_ptr fn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t outputs_count,
    element_evaluator_options opts)
{
    evaluator_ctx ectx = { inputs, inputs_count, std::move(opts) };
    size_t outputs_written = 0;
    return do_evaluate(ectx, fn, outputs, outputs_count, outputs_written);
}

element_value element_evaluate_unary(element_unary::op op, element_value a)
{
    switch (op) {
    case element_unary::op::abs:   return std::fabs(a);
    case element_unary::op::acos:  return std::acos(a);
    case element_unary::op::asin:  return std::asin(a);
    case element_unary::op::atan:  return std::atan(a);
    case element_unary::op::ceil:  return std::ceil(a);
    case element_unary::op::cos:   return std::cos(a);
    case element_unary::op::floor: return std::floor(a);
    case element_unary::op::ln:    return std::log(a);
    case element_unary::op::sin:   return std::sin(a);
    case element_unary::op::tan:   return std::tan(a);
    default: assert(false);        return static_cast<element_value>(std::nan(""));
    }
}

element_value element_evaluate_binary(element_binary::op op, element_value a, element_value b)
{
    switch (op) {
    case element_binary::op::add:   return a + b;
    case element_binary::op::atan2: return std::atan2(a, b);
    case element_binary::op::div:   return a / b;
    case element_binary::op::log:   return std::log10(a) / std::log10(b); // TODO: optimised pseudo-ops for log10 et al?
    case element_binary::op::max:   return (std::max)(a, b);
    case element_binary::op::min:   return (std::min)(a, b);
    case element_binary::op::mul:   return a * b;
    case element_binary::op::pow:   return std::pow(a, b);
    case element_binary::op::rem:   return std::remainder(a, b);
    case element_binary::op::sub:   return a - b;
    default: assert(false);         return static_cast<element_value>(std::nan(""));
    }
}