#include "intermediaries.hpp"

//STD
#include <cassert>

//LIBS
#include <fmt/format.h>

#include "errors.hpp"

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

        auto func = std::dynamic_pointer_cast<function_declaration>(declarer->our_scope->find(name, false));
        //todo: not exactly working type checking, good enough for now though
        const bool has_inputs = func && func->has_inputs();
        const bool has_type = has_inputs && func->inputs[0].annotation.get();
        const bool types_match = has_type && func->inputs[0].annotation->name.value == declarer->name.value;

        if (types_match)
        {
            std::vector<std::shared_ptr<object>> args = { const_cast<struct_instance*>(this)->shared_from_this() };
            return std::make_shared<function_instance>(func.get(), context.stack, std::move(args));
        }

        //todo: instead of relying on generic error handling for nullptr, build a specific error
        if (!func)
            return nullptr;

        if (!has_inputs)
            return build_error(source_info, error_message_code::instance_function_cannot_be_nullary,
                func->typeof_info(), typeof_info());

        if (!has_type)
            return build_error(source_info, error_message_code::is_not_an_instance_function,
                func->typeof_info(), typeof_info(), func->inputs[0].name.value);

        if (!types_match)
            return build_error(source_info, error_message_code::is_not_an_instance_function,
                func->typeof_info(), typeof_info(),
                func->inputs[0].name.value, func->inputs[0].annotation->name.value, declarer->name.value);

        //did we miss an error that we need to handle?
        assert(false);
        return nullptr;
    }

    std::shared_ptr<object> struct_instance::compile(const compilation_context& context, const source_information& source_info) const
    {
        return const_cast<struct_instance*>(this)->shared_from_this();
    }

    //function_instance
    function_instance::function_instance(const function_declaration* declarer, call_stack stack)
        : declarer(declarer)
        , stack(std::move(stack))
    {
    }

    function_instance::function_instance(const function_declaration* declarer, call_stack stack, std::vector<std::shared_ptr<object>> args)
        : declarer(declarer)
        , stack(std::move(stack))
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

        if (context.stack.is_recursive(declarer))
            return context.stack.build_recursive_error(declarer, context, source_info);

        //todo: the callstack doesn't need to swap, what does is the capture stack, which is hackily implemented as part of the callstack right now
        std::swap(stack, context.stack);
        context.stack.push(declarer, std::move(provided_arguments));
        auto ret = declarer->body->compile(context, source_info);
        context.stack.pop();
        std::swap(stack, context.stack);
        return ret;
    }
}
