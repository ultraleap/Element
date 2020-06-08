#pragma once

#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct declaration : public element_object
	{
        explicit declaration(element_object* parent) : element_object(parent)
        {
        }
    };
}
