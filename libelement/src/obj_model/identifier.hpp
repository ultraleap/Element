#pragma once

#include "element_object.hpp"

namespace element::object_model
{
    struct identifier : public element_object
	{
        explicit identifier(element_object* parent) : element_object(parent)
        {
        }
    };
}
