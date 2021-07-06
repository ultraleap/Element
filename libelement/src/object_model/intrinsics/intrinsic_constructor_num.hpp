#pragma once

//SELF
#include "intrinsic_function.hpp"

namespace element
{
class intrinsic_constructor_num final : public intrinsic_function
{
public:
    DECLARE_TYPE_ID();

    intrinsic_constructor_num();

    [[nodiscard]] object_const_shared_ptr call(
        const compilation_context& context,
        std::vector<object_const_shared_ptr> compiled_args,
        const source_information& source_info) const override;

    [[nodiscard]] std::string get_name() const override;
};
} // namespace element