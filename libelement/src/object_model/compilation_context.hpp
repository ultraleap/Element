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
        [[nodiscard]] const scope* get_compiler_scope() const { return compiler_scope.get(); }
        [[nodiscard]] const element_log_ctx* get_logger() const;

        mutable call_stack calls;
        mutable capture_stack captures;
        mutable source_information source_info;

        element_interpreter_ctx* interpreter;

    private:
        const scope* global_scope;

        //Used to store declarations that aren't in user code, but which we need to store somewhere
        //e.g. a declaration for the compiler-provided list indexer when a user creates a List using the intrinsic function list
        std::unique_ptr<scope> compiler_scope;
    };
}