#pragma once

#include <vector>
#include <cstdlib>

#include "element/interpreter.h"
#include "etree/expressions.hpp"

struct evaluator_ctx;
element_result element_evaluate(
    element_interpreter_ctx& context,
    expression_const_shared_ptr fn,
    const std::vector<element_value>& inputs,
    std::vector<element_value>& outputs,
    element_evaluator_options opts);

element_result element_evaluate(
    element_interpreter_ctx& context,
    expression_const_shared_ptr fn,
    const element_value* inputs, size_t inputs_count,
    element_value* outputs, size_t& outputs_count,
    element_evaluator_options opts);

element_value element_evaluate_nullary(element_expression_nullary::op op);
element_value element_evaluate_unary(element_expression_unary::op op, element_value a);
element_value element_evaluate_binary(element_expression_binary::op op, element_value a, element_value b);
element_value element_evaluate_if(element_value predicate, element_value if_true, element_value if_false);
element_value element_evaluate_for(evaluator_ctx& context, const expression_const_shared_ptr& initial, const expression_const_shared_ptr& condition, const expression_const_shared_ptr&);
expression_const_shared_ptr element_evaluate_select(element_value selector, std::vector<expression_const_shared_ptr> options);