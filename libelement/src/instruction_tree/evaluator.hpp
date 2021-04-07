#pragma once

#include <vector>
#include <cstdlib>

#include "element/interpreter.h"
#include "instruction_tree/instructions.hpp"

struct element_evaluator_ctx;

element_result element_evaluate(
    element_evaluator_ctx& context,
    const element::instruction_const_shared_ptr& fn,
    const std::vector<element_value>& inputs,
    std::vector<element_value>& outputs);

element_result element_evaluate(
    element_evaluator_ctx& context,
    const element::instruction_const_shared_ptr& fn,
    const element_value* inputs,
    size_t inputs_count,
    element_value* outputs,
    size_t& outputs_count);

element_value element_evaluate_nullary(element::instruction_nullary::op op);
element_value element_evaluate_unary(element::instruction_unary::op op, element_value a);
element_value element_evaluate_binary(element::instruction_binary::op op, element_value a, element_value b);
element_value element_evaluate_if(element_value predicate, element_value if_true, element_value if_false);
std::vector<element_value> element_evaluate_for(element_evaluator_ctx& context, const element::instruction_const_shared_ptr& initial, const element::instruction_const_shared_ptr& condition, const element::instruction_const_shared_ptr&);
std::size_t element_evaluate_select(element_value selector, size_t option_count);