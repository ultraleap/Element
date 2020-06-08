#pragma once

#include "expression.hpp"
#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct call_expression : public expression
	{
	    explicit call_expression(element_object* parent) : expression(parent)
        {
        }
    };
}
