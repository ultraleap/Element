#pragma once

//STD
#include <memory>

//SELF
#include "common_internal.hpp"

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
    not_, //reserved keyword
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
    and_, //reserved keyword
    or_, //reserved keyword

	//comparison
	eq,
	neq,
	lt,
	leq,
	gt,
	geq
};