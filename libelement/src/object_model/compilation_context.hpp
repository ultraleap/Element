#pragma once

//SELF
#include "call_stack.hpp"
#include "capture_stack.hpp"
#include "interpreter_internal.hpp"

//STD
#include <set>

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

    struct boundary_info
    {
        size_t size = 0;
        std::set<std::shared_ptr<const instruction_input>> inputs;
    };

    mutable std::vector<boundary_info> boundaries;

    size_t total_boundary_size_at_index(size_t index) const
    {
        size_t total_size = 0;
        for (size_t i = 0; i < index + 1; ++i)
            total_size = boundaries[i].size;
        return total_size;
    }

    element_interpreter_ctx* interpreter;

private:
    const scope* global_scope;
    //Used to store declarations that aren't in user code, but which we need to store somewhere
    //e.g. a declaration for the compiler-provided list indexer when a user creates a List using the intrinsic function list
    std::unique_ptr<scope> compiler_scope;
};
} // namespace element