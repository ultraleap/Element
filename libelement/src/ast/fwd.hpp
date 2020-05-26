#pragma once

#include <memory>
#include "common_internal.hpp"

struct element_scope;
using scope_unique_ptr = std::unique_ptr<element_scope>;

struct element_constraint;
using constraint_shared_ptr = std::shared_ptr<element_constraint>;
using constraint_const_shared_ptr = std::shared_ptr<const element_constraint>;

struct element_type;
using type_shared_ptr = std::shared_ptr<element_type>;
using type_const_shared_ptr = std::shared_ptr<const element_type>;

struct element_function;
using function_shared_ptr = std::shared_ptr<element_function>;
using function_const_shared_ptr = std::shared_ptr<const element_function>;

enum class element_nullary_op {
	//num
	nan,
	positive_infinity,
	negative_infinity,

	//boolean
    true_value,
    false_value,
};

enum class element_unary_op
{
	//num
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

	//boolean
    not,
};

enum class element_binary_op
{
	//num
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

	//boolean
    and,
    or,

	//comparison
	eq,
	neq,
	lt,
	leq,
	gt,
	geq
};