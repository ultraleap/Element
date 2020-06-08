#pragma once

#include "declaration.hpp"
#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct intrinsic_constraint_declaration : public declaration
	{
        explicit intrinsic_constraint_declaration(element_object* parent) : declaration(parent)
        {
        }
    };
}
