#pragma once

//SELF
#include "intrinsic_function.hpp"

namespace element
{
    class intrinsic_unary final : public intrinsic_function
    {
    public:
        DECLARE_TYPE_ID();

        explicit intrinsic_unary(element_unary_op operation, type_const_ptr return_type, type_const_ptr argument_type);

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override;
        [[nodiscard]] element_unary_op get_operation() const { return operation; }

    private:
        element_unary_op operation;
        //TODO: this might need to be a constraint_const_shared_ptr
        type_const_ptr argument_type;
    };
}