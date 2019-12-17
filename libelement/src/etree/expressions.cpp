#include "etree/expressions.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include "interpreter_internal.hpp"

DEFINE_TYPE_ID(element_constant,   1U << 0);
DEFINE_TYPE_ID(element_input,      1U << 1);
DEFINE_TYPE_ID(element_structure,  1U << 2);
DEFINE_TYPE_ID(element_unary,      1U << 3);
DEFINE_TYPE_ID(element_binary,     1U << 4);
DEFINE_TYPE_ID(element_expr_group, 1U << 5);
