#include "intermediaries.hpp"

#include <cassert>

namespace element
{
    //struct_instance
    struct_instance::struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<object>>& expressions)
        : declarer{ declarer }
    {
        //TODO: JM - variadics
        assert(declarer->inputs.size() == expressions.size());
        for (size_t i = 0; i < declarer->inputs.size(); ++i)
        {
            //fields.emplace(declarer->inputs[i].identifier, expressions[i]);
        }
    }

    std::shared_ptr<object> struct_instance::index(const compilation_context& context, const identifier& identifier) const
    {
        //this is how we do partial application. if we index a struct instance and find it's an instance function
        //then we create a function_instance of that function, with ourselves as the first provided argument
        //when we return that function_instance, either the next expression is a call which fills the remaining arguments and then calls it
        //or we just return it/store it, to be used later
        return nullptr;
    }

    std::string struct_instance::to_string() const
    {
        return "Instance:" + declarer->to_string();
    }

    //function_instance
    function_instance::function_instance(const function_declaration* declarer, call_stack stack, std::vector<std::shared_ptr<element_expression>> args)
        : declarer(declarer)
        , stack(std::move(stack))
        , provided_arguments(std::move(args))
    {
    }

    std::string function_instance::to_string() const
    {
        return declarer->location() + ":FunctionInstance";
    }

    std::shared_ptr<object> function_instance::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        compiled_args.insert(compiled_args.begin(), provided_arguments.begin(), provided_arguments.end());

        if (compiled_args.size() == declarer->inputs.size())
        {
            //we have the exact arguments we need, so now we just need to perform the call
            //we also need to swap the callstacks so that the call can use the one at the point the instance was created
            std::swap(stack, context.stack);
            auto ret = declarer->call(context, compiled_args);
            //we then swap them back, in case our instance is called multiple times.
            //Technically the call we did could have modified the callstack, but if it's behaving correctly it will be identical due to popping frames
            std::swap(stack, context.stack);
            return ret;
        }

        assert(false);
        return nullptr;
    }

    std::shared_ptr<object> function_instance::compile(const compilation_context& context) const
    {
        //the last thing in an expression was an instance function with everything fully applied
        if (provided_arguments.size() == declarer->inputs.size())
            return call(context, {});

        return const_cast<function_instance*>(this)->shared_from_this();
    }
}
