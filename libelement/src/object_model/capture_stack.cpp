#include "capture_stack.hpp"

//SELF
#include "call_stack.hpp"
#include "scope.hpp"
#include "declarations/declaration.hpp"

using namespace element;

capture_stack::capture_stack(
    const declaration* function,
    const call_stack& calls)
{
    //build up a list of declarations with their captures, using each parent scope of the passed in declaration
    const scope* s = function->our_scope.get();
    while (s)
    {
        const declaration* decl = s->declarer;

        auto found_it = std::find_if(std::rbegin(calls.frames), std::rend(calls.frames),
            [decl](const auto& frame) {
                return frame.function == decl;
        });

        if (found_it != std::rend(calls.frames))
            frames.emplace_back(frame{ found_it->function, found_it->compiled_arguments });
        
        s = s->get_parent_scope();
    }
}

void capture_stack::push(const declaration* function, std::vector<object_const_shared_ptr> compiled_arguments)
{
    frames.emplace_back(frame{function, std::move(compiled_arguments)});
}

void capture_stack::pop()
{
    frames.pop_back();
}

object_const_shared_ptr capture_stack::find(const scope* s, const identifier& name,
                                                    const compilation_context& context,
                                                    const source_information& source_info)
{
    while (s)
    {
        const auto found = s->find(name, false);
        if (found)
            return found->compile(context, source_info);

        auto found_it = std::find_if(std::begin(frames), std::end(frames), 
            [function = s->declarer](const auto& frame) {
                return function == frame.function;
        });

        if (found_it != frames.end())
        {
            const auto* func = found_it->function;
            for (unsigned i = 0; i < func->inputs.size(); ++i)
            {
                const auto& input = func->inputs[i];
                if (input.get_name() == name.value)
                    return found_it->compiled_arguments[i];
            }
        }

        s = s->get_parent_scope();
    }

    return nullptr;
}