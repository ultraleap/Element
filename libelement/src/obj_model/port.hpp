#pragma once

#include <vector>

#include "element_object.hpp"

namespace element::object_model
{
    struct port : public element_object
	{
        explicit port(element_object* parent) : element_object(parent)
        {
        }
    };
}
