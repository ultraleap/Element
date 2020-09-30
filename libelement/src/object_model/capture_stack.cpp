#include "capture_stack.hpp"

//SELF
#include "call_stack.hpp"
#include "scope.hpp"
#include "declarations/declaration.hpp"

using namespace element;

void capture_stack::push(const scope* local_scope, const std::vector<port>* parameters, std::vector<object_const_shared_ptr> compiled_arguments)
{
    frames.emplace_back(frame{local_scope, parameters, std::move(compiled_arguments)});
}

void capture_stack::push(const declaration& declaration, std::vector<object_const_shared_ptr> compiled_arguments)
{
    push(declaration.our_scope.get(), &declaration.get_inputs(), std::move(compiled_arguments));
}

void capture_stack::pop()
{
    frames.pop_back();
}

object_const_shared_ptr capture_stack::find(const scope* local_scope,
                                            const identifier& name,
                                            const compilation_context& context,
                                            const source_information& source_info) const
{
    const scope* current_scope = local_scope;

    while (current_scope)
    {
        //check the scope and see if it's in there
        const auto* found = current_scope->find(name, false);
        if (found)
            return found->compile(context, source_info);

        //if it's not, then find the scope in our stack
        auto found_it = std::find_if(std::rbegin(frames), std::rend(frames),
            [current_scope](const auto& frame) {
            return current_scope == frame.current_scope;
        });

        //if the scope is in our stack, check the parameters if there are any
        if (found_it != frames.rend() && found_it->parameters)
        {
            const auto& parameters = *found_it->parameters;
            for (unsigned i = 0; i < parameters.size(); ++i)
            {
                const auto& param = parameters[i];
                if (param.get_name() == name.value)
                    return found_it->compiled_arguments[i];
            }
        }

        current_scope = current_scope->get_parent_scope();
    }

    return nullptr;
}