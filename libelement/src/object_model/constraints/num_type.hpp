#pragma once

//SELF
#include "type.hpp"

namespace element
{
    class num_type : public type
    {
    public:
        DECLARE_TYPE_ID();
        num_type() : type(type_id, name, nullptr) {}

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
                                                    const identifier& name,
                                                    const source_information& source_info) const override;

    private:
        static identifier name;
    };
}