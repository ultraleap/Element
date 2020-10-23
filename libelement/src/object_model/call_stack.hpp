#pragma once

//STD
#include <vector>

//SELF
#include "fwd.hpp"

namespace element
{
    class call_stack
    {
    public:
        struct frame
        {
            const declaration* function;
            std::vector<object_const_shared_ptr> compiled_arguments;
        };

        frame& push(const declaration* function, std::vector<object_const_shared_ptr> compiled_arguments);
        void pop();

        [[nodiscard]] bool is_recursive(const declaration* declaration) const;
        [[nodiscard]] std::shared_ptr<error> build_recursive_error(
            const declaration* decl,
            const compilation_context& context,
            const source_information& source_info);

        //todo: private
        std::vector<frame> frames;
    };
} // namespace element