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
        std::shared_ptr<const function_instance> function;
        std::vector<object_const_shared_ptr> compiled_arguments;
    };

    call_stack();
    frame& push(std::shared_ptr<const function_instance> function, std::vector<object_const_shared_ptr> compiled_arguments);
    void pop();

    [[nodiscard]] unsigned int recursive_calls(const function_instance* function) const;
    [[nodiscard]] std::shared_ptr<error> build_recursive_error(
        const function_instance* function,
        const compilation_context& context,
        const source_information& source_info);

    //todo: private
    std::vector<frame> frames;
};
} // namespace element