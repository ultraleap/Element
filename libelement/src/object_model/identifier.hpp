#pragma once

//STD
#include <string>

namespace element
{
    //todo: do we actually want to keep and use this type? currently we're just accessing .value everywhere, if we want to keep this, let's properly wrap std::string
    class identifier
    {
    public:
        identifier() = default;

        identifier(std::string value)
            : value{ std::move(value) }
        {
        }

        static identifier return_identifier;

        std::string value;

        bool operator <(const identifier& rhs) const
        {
            return value < rhs.value;
        }
    };
}