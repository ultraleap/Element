#include "call_stack.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "declarations/function_declaration.hpp"
#include "intermediaries/function_instance.hpp"
#include "compilation_context.hpp"
#include "error_map.hpp"

using namespace element;

call_stack::call_stack()
{
    frames.reserve(100);
}

call_stack::frame& call_stack::push(std::shared_ptr<const function_instance> function, std::vector<object_const_shared_ptr> compiled_arguments)
{
    return frames.emplace_back(frame{ std::move(function), std::move(compiled_arguments) });
}

void call_stack::pop()
{
    frames.pop_back();
}

unsigned int call_stack::recursive_calls(const function_instance* function) const
{
    auto count = 0;

    const bool check_recursion = !function->declarer->is_intrinsic();
    if (!check_recursion)
        return count;

    for (auto it = frames.rbegin(); it != frames.rend(); ++it)
    {
        if (it->function->declarer == function->declarer)
            count++;
    }

    return count;
}

std::shared_ptr<error> call_stack::build_recursive_error(
    const function_instance* function,
    const compilation_context& context,
    const source_information& source_info)
{
    std::string trace;

    for (auto it = context.calls.frames.rbegin(); it < context.calls.frames.rend(); ++it)
    {
        auto& func = it->function;

        std::string params;
        for (unsigned i = 0; i < func->declarer->inputs.size(); ++i)
        {
            const auto& input = func->declarer->inputs[i];
            params += fmt::format("{}{} = {}",
                                  input.get_name(),
                                  input.has_annotation() ? ":" + input.get_annotation()->to_string() : "",
                                  it->compiled_arguments[i]->to_string());

            if (i != func->declarer->inputs.size() - 1)
                params += ", ";
        }

        std::string ret;
        if (func->declarer->output.get_annotation() && !func->declarer->output.get_annotation()->to_string().empty())
            ret = ":" + func->declarer->output.get_annotation()->to_string();

        trace += fmt::format("{}:{} at {}({}){}",
                             func->source_info.filename,
                             func->source_info.line,
                             func->declarer->name.value,
                             params,
                             ret);

        if (func->declarer == function->declarer)
            trace += " <-- here";

        if (it != context.calls.frames.rend() - 1)
            trace += "\n";
    }

    return build_error_and_log(context, source_info, error_message_code::recursion_detected, function->to_string(), trace);
}