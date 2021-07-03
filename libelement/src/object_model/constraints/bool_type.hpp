#pragma once

//SELF
#include "type.hpp"

namespace element
{
class bool_type : public type
{
public:
    DECLARE_TYPE_ID();
    bool_type()
        : type(type_id, name, nullptr)
    {}

    [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const override;

    [[nodiscard]] bool is_constant() const override;

private:
    static identifier name;
    mutable bool cached = false;
    mutable const declaration* cached_declaration = nullptr;
};
} // namespace element