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
            const scope* current_scope;
            const std::vector<port>* parameters;
            std::vector<object_const_shared_ptr> compiled_arguments;
        };

        capture_stack() = default;

        void push(const scope* local_scope, const std::vector<port>* parameters, std::vector<object_const_shared_ptr> compiled_arguments);
        void push(const declaration& declaration, std::vector<object_const_shared_ptr> compiled_arguments);
        void pop();
        [[nodiscard]] object_const_shared_ptr find(const scope* local_scope,
                                                   const identifier& name,
                                                   const compilation_context& context,
                                                   const source_information& source_info) const;

        //todo: private
        std::vector<frame> frames;
    };
} // namespace element