#pragma once

#include "declaration.hpp"
#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct namespace_declaration : public declaration
	{
        explicit namespace_declaration(element_object* parent) : declaration(parent)
        {
        }
    };
}
