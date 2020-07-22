#include "intermediaries.hpp"

//STD
#include <cassert>

//LIBS
#include <fmt/format.h>

#include "errors.hpp"

namespace element
{
    struct_instance::struct_instance(const struct_declaration* declarer)
        : declarer(declarer)
    {
    }

    //struct_instance
    struct_instance::struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<object>>& expressions)
        : declarer(declarer)
    {
        //TODO: JM - variadics
        assert(declarer->inputs.size() == expressions.size());
        for (size_t i = 0; i < declarer->inputs.size(); ++i)
        {
            fields.emplace(declarer->inputs[i].name.value, expressions[i]);
        }
    }

    std::shared_ptr<object> struct_instance::index(const compilation_context& context, const identifier& name, const source_information& source_info) const
    {
        //this is how we do partial application. if we index a struct instance and find it's an instance function
        //then we create a function_instance of that function, with ourselves as the first provided argument
        //when we return that function_instance, either the next expression is a call which fills the remaining arguments and then calls it
        //or we just return it/store it, to be used later
        const auto found_field = fields.find(name.value);

        //found it as a field
        if (found_field != fields.end())
            return found_field->second;

        return index_type(declarer, const_cast<struct_instance*>(this)->shared_from_this(), context, name, source_info);
    }

    std::shared_ptr<object> struct_instance::compile(const compilation_context& context, const source_information& source_info) const
    {
        return const_cast<struct_instance*>(this)->shared_from_this();
    }

    //function_instance
    function_instance::function_instance(const function_declaration* declarer, capture_stack captures)
        : declarer(declarer)
        , captures(std::move(captures))
    {
    }

    function_instance::function_instance(const function_declaration* declarer, capture_stack captures, std::vector<std::shared_ptr<object>> args)
        : declarer(declarer)
        , captures(std::move(captures))
        , provided_arguments(std::move(args))
    {
    }

    std::shared_ptr<object> function_instance::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        provided_arguments.insert(provided_arguments.end(), compiled_args.begin(), compiled_args.end());
        return compile(context, source_info);
    }

    std::shared_ptr<object> function_instance::compile(const compilation_context& context, const source_information& source_info) const
    {
        //todo: error for assert
        assert(provided_arguments.size() <= declarer->inputs.size());

        if (provided_arguments.size() != declarer->inputs.size())
            return const_cast<function_instance*>(this)->shared_from_this();

        if (context.calls.is_recursive(declarer))
            return context.calls.build_recursive_error(declarer, context, source_info);

        //todo: capture stack is recreated based on the call stack, so it needs to swap, but realistically we should be swapping capture stacks
        //TODO: ARGH!!! Can we simplify this...
        context.calls.push(declarer, provided_arguments);
        captures.frames.emplace_back(capture_stack::frame{declarer, provided_arguments });

        std::swap(captures, context.captures);
        auto ret = declarer->body->compile(context, source_info);
        std::swap(captures, context.captures);

        captures.frames.pop_back();
        context.calls.pop();
        return ret;
    }
}
