#pragma once

#include "expression.hpp"
#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct indexing_expression : public expression
	{
        explicit indexing_expression(element_object* parent) : expression(parent)
        {
        }
    };
}
