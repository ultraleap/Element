#pragma once

#include <utility>


#include "obj_model/element_object.hpp"

namespace element
{
    struct expression;

    struct type_annotation : element_object
	{
        identifier identifier;

        //TODO: This should be an expression, not an identifier
        type_annotation(element::identifier identifier)
            : identifier{std::move(identifier)}
        { 
        }

        [[nodiscard]] std::string to_code(int depth) const override;
    };
}
