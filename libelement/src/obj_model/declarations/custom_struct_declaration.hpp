#pragma once

#include "declaration.hpp"
#include "obj_model/element_object.hpp"

namespace element::object_model
{
    struct custom_struct_declaration : public declaration
	{
        explicit custom_struct_declaration(element_object* parent) : declaration(parent)
        {
        }
    };
}
