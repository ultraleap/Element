#pragma once

#include "element_object.hpp"

namespace element::object_model
{
    struct literal : public element_object
	{
        explicit literal(element_object* parent) : element_object(parent)
        {
        }
    };
}
