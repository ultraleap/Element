#pragma once

//SELF
#include "typeutil.hpp"
#include "constraint.hpp"

namespace element
{
class any_constraint : public constraint
{
public:
    DECLARE_TYPE_ID();
    any_constraint()
        : constraint(type_id, nullptr)
    {}
};
} // namespace element