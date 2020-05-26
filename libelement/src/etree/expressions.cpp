#include "expressions.hpp"

//todo: why do we use bits instead of normal numbers?
DEFINE_TYPE_ID(element_expression_constant,        1U << 0);
DEFINE_TYPE_ID(element_expression_input,           1U << 1);
DEFINE_TYPE_ID(element_expression_structure,       1U << 2);
DEFINE_TYPE_ID(element_expression_nullary,         1U << 3);
DEFINE_TYPE_ID(element_expression_unary,           1U << 4);
DEFINE_TYPE_ID(element_expression_binary,          1U << 5);
DEFINE_TYPE_ID(element_expression_group,           1U << 6);
DEFINE_TYPE_ID(element_expression_unbound_arg,     1U << 7);