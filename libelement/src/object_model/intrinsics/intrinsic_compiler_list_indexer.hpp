#pragma once

//SELF
#include "intrinsic_function.hpp"

namespace element
{
//TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
class intrinsic_compiler_list_indexer final : public intrinsic_function
{
public:
    DECLARE_TYPE_ID();

    intrinsic_compiler_list_indexer();

    [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
        const source_information& source_info) const override;
};
} // namespace element