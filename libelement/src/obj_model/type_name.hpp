#pragma once

#include "element_object.hpp"

namespace element::object_model
{
    struct type_name : public element_object
	{
        explicit type_name(element_object* parent) : element_object(parent)
        {
        }
    };
}
