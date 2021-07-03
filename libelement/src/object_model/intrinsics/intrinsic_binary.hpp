#pragma once

//SELF
#include "intrinsic_function.hpp"

namespace element
{
class intrinsic_binary final : public intrinsic_function
{
public:
    DECLARE_TYPE_ID();

    intrinsic_binary(element_binary_op operation, type_const_ptr return_type, type_const_ptr first_argument_type, type_const_ptr second_argument_type);

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override;

    [[nodiscard]] element_binary_op get_operation() const { return operation; }

private:
    element_binary_op operation;
    //TODO: this might need to be a constraint_const_shared_ptr
    type_const_ptr first_argument_type;
    //TODO: this might need to be a constraint_const_shared_ptr
    type_const_ptr second_argument_type;
};
} // namespace element