#pragma once

//STD
#include <utility>

//SELF
#include "obj_model/object.hpp"

namespace element
{
    class type_annotation : public object
    {
    public:
        //TODO: This should be an expression, not an identifier
        type_annotation(identifier name)
            : name{std::move(name)}
        {
        }

        [[nodiscard]] std::string to_code(int depth) const override;

        identifier name;

    private:
    };
}
