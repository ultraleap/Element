#pragma once

#include "expression.hpp"
#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct lambda_expression : public expression
	{
        explicit lambda_expression(element_object* parent) : expression(parent)
        {
        }
    };
}
