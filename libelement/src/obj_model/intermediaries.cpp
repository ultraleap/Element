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

        if (func)
        {
            if (!has_inputs)
            {
                return build_error(source_info, error_message_code::instance_function_cannot_be_nullary, 
                    func->typeof_info(), typeof_info());
            }

            if (!has_type)
            {
                return build_error(source_info, error_message_code::is_not_an_instance_function,
                    func->typeof_info(), typeof_info(), func->inputs[0].name.value);
            }

            if (!types_match)
            {
                return build_error(source_info, error_message_code::is_not_an_instance_function,
                    func->typeof_info(), typeof_info(), func->inputs[0].name.value, func->inputs[0].annotation->name.value, declarer->name.value);
            }
        }

        //todo: instead of relying on generic error handling for nullptr, build a specific error
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
        , provided_arguments(std::move(args))
        , stack(std::move(stack))
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
        if (provided_arguments.size() == declarer->inputs.size())
        {
            //we have the exact arguments we need, so now we just need to perform the call
            //we also need to swap the call stacks so that the call can use the one at the point the instance was created
            std::swap(stack, context.stack);

            if (context.is_recursive(declarer))
                return declarer->is_recursive(context);

            call_stack::frame frame;
            frame.function = declarer;
            frame.compiled_arguments = std::move(provided_arguments);
            context.stack.frames.emplace_back(std::move(frame));

            if (!declarer->body)
            {
                return std::make_shared<error>(
                    fmt::format("failed at {}. scope bodied functions must contain a return function.\n", typeof_info()),
                    ELEMENT_ERROR_MISSING_FUNCTION_BODY,
                    source_info);
            }

            //arguments are in the callstack
            std::shared_ptr<object> ret = declarer->body->compile(context, source_info);

            context.stack.frames.pop_back();

            //we then swap them back, in case our instance is called multiple times.
            //Technically the call we did could have modified the call stack, but if it's behaving correctly it will be identical due to popping frames
            std::swap(stack, context.stack);
            return ret;
        }

        assert(provided_arguments.size() < declarer->inputs.size());

        return const_cast<function_instance*>(this)->shared_from_this();
    }
}
