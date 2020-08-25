#pragma once

//SELF
#include "constraint.hpp"

namespace element
{
    class user_function_constraint : public constraint {
    public:
        DECLARE_TYPE_ID();
        user_function_constraint(const declaration* declarer) : constraint(type_id, declarer) {}

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
    };
}