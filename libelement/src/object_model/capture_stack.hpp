#pragma once

//STD
#include <vector>

//SELF
#include "fwd.hpp"

namespace element
{
    class capture_stack
    {
    public:
        struct frame
        {
            const declaration* function;
            std::vector<object_const_shared_ptr> compiled_arguments;
        };

        capture_stack() = default;
        capture_stack(const declaration* function, const call_stack& calls);

        void push(const declaration* function, std::vector<object_const_shared_ptr> compiled_arguments);
        void pop();
        [[nodiscard]] object_const_shared_ptr find(const scope* s,
                                                   const identifier& name,
                                                   const compilation_context& context,
                                                   const source_information& source_info);

        //todo: private
        std::vector<frame> frames;
    };
}