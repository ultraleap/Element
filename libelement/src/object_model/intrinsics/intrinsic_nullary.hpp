#pragma once

//SELF
#include "intrinsic_function.hpp"

namespace element
{
class intrinsic_nullary final : public intrinsic_function
{
public:
    DECLARE_TYPE_ID();

    explicit intrinsic_nullary(element_nullary_op operation, type_const_ptr return_type);

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override;
    [[nodiscard]] element_nullary_op get_operation() const { return operation; }

private:
    element_nullary_op operation;
};
} // namespace element