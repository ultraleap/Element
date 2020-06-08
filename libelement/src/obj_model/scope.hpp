#pragma once

#include "element_object.hpp"

namespace element::object_model
{
    struct scope : public element_object
	{
        explicit scope(element_object* parent) : element_object(parent)
        {
        }
    };
}
