#pragma once

#include <memory>
#include <vector>
#include "common_internal.hpp"

struct element_scope;
using scope_unique_ptr = std::unique_ptr<element_scope>;

struct element_type_constraint;
using type_constraint_shared_ptr = std::shared_ptr<element_type_constraint>;
using type_constraint_const_shared_ptr = std::shared_ptr<const element_type_constraint>;

struct element_type;
using type_shared_ptr = std::shared_ptr<element_type>;
using type_const_shared_ptr = std::shared_ptr<const element_type>;

struct element_function;
using function_shared_ptr = std::shared_ptr<element_function>;
using function_const_shared_ptr = std::shared_ptr<const element_function>;

enum class element_unary_op
{
    sin,
    cos,
    tan,
    asin,
    acos,
    atan,
    ln,
    abs,
    ceil,
    floor,
};

enum class element_binary_op
{
    add,
    sub,
    mul,
    div,
    rem,
    pow,
    min,
    max,
    log,
    atan2,
};
