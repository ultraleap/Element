#pragma once

//SELF
#include "call_stack.hpp"
#include "capture_stack.hpp"
#include "interpreter_internal.hpp"

namespace element
{
    class compilation_context
    {
    public:
        explicit compilation_context(const scope* scope, element_interpreter_ctx* interpreter);

        [[nodiscard]] const scope* get_global_scope() const { return global_scope; }
        [[nodiscard]] const element_log_ctx* get_logger() const;

        mutable call_stack calls;
        mutable capture_stack captures;
        mutable source_information source_info;

        element_interpreter_ctx* interpreter;

    private:
        const scope* global_scope;
    };
}