#pragma once

//SELF
#include "constraint.hpp"

namespace element
{
    class type : public constraint
    {
    public:
        DECLARE_TYPE_ID();

        static const type_const_unique_ptr num; // the absolute unit
        static const type_const_unique_ptr boolean;

        [[nodiscard]] identifier get_identifier() const { return name; }
        [[nodiscard]] std::string get_name() const final { return name.value; }

    protected:
        type(element_type_id id, identifier name, const declaration* declarer)
            : constraint(type_id, declarer)
            , name(std::move(name))
        {
        }

        identifier name;
    };
} // namespace element