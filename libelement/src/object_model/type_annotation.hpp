#pragma once

//STD
#include <utility>

//SELF
#include "fwd.hpp"
#include "source_information.hpp"
#include "identifier.hpp"

namespace element
{
    class type_annotation final
    {
    public:
        //TODO: This should be an expression, not an identifier
        explicit type_annotation(identifier name)
            : name{std::move(name)}
        {
        }

        [[nodiscard]] std::string to_code(const int depth) const;
        [[nodiscard]] const std::string& to_string() const { return name.value; };

        source_information source_info;

    private:
        identifier name;
    };
}
