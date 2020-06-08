#pragma once

#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct expression : public element_object
	{
        explicit expression(element_object* parent) : element_object(parent)
        {
        }
    };
}
