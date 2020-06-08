#pragma once

#include "element_object.hpp"

namespace element::object_model
{
    struct expression_list : public element_object
	{
        explicit expression_list(element_object* parent) : element_object(parent)
        {
        }
    };
}
