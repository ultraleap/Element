#pragma once

//SELF
#include "intrinsic_function.hpp"

namespace element
{
class intrinsic_for final : public intrinsic_function
{
public:
    DECLARE_TYPE_ID();

    intrinsic_for();

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override;
};
} // namespace element