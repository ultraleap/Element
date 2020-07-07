#pragma once

//STD
#include <utility>

//SELF
#include "object.hpp"

namespace element
{
    class type_annotation : public object
    {
    public:
        //TODO: This should be an expression, not an identifier
        explicit type_annotation(identifier name)
            : name{std::move(name)}
        {
        }

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        identifier name;

    private:
    };
}
