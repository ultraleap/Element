#pragma once

//STD
#include <utility>

//SELF
#include "obj_model/object.hpp"

namespace element
{
    struct type_annotation : object
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
