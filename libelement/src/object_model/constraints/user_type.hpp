#pragma once

//SELF
#include "type.hpp"

namespace element
{
    class user_type : public type
    {
    public:
        DECLARE_TYPE_ID();
        user_type(identifier name, const declaration* declarer)
            : type(type_id, name, declarer)
            , declarer(declarer)
            , name(std::move(name))
        {}

    private:
        const declaration* declarer;
        identifier name;
    };
} // namespace element