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
    function_instance::function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info)
        : declarer(declarer)
        , captures(std::move(captures))
    {
        this->source_info = std::move(source_info);
    }

    function_instance::function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info, std::vector<std::shared_ptr<object>> args)
        : declarer(declarer)
        , captures(std::move(captures))
        , provided_arguments(std::move(args))
    {
        this->source_info = std::move(source_info);
    }

    std::shared_ptr<object> function_instance::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        compiled_args.insert(std::begin(compiled_args), std::begin(provided_arguments), std::end(provided_arguments));

        //element doesn't allow partial application generally
        //todo: more helpful information than the number of arguments expected
        if (compiled_args.size() < declarer->inputs.size())
            return build_error_and_log(context, source_info, error_message_code::not_enough_arguments,
                declarer->name.value, compiled_args.size(), declarer->inputs.size());

        //todo: more helpful information than the number of arguments expected
        if (compiled_args.size() > declarer->inputs.size())
            return build_error_and_log(context, source_info, error_message_code::too_many_arguments,
                declarer->name.value, compiled_args.size(), declarer->inputs.size());

        if (context.calls.is_recursive(declarer))
            return context.calls.build_recursive_error(declarer, context, source_info);

        context.calls.push(declarer, compiled_args);
        captures.push(declarer, compiled_args);

        std::swap(captures, context.captures);
        auto element = declarer->body->compile(context, source_info);
        std::swap(captures, context.captures);

        captures.pop();
        context.calls.pop();
        return element;
    }

    std::shared_ptr<object> function_instance::compile(const compilation_context& context, const source_information& source_info) const
    {
        if (provided_arguments.size() == declarer->inputs.size())
            return call(context, {}, source_info);

        return const_cast<function_instance*>(this)->shared_from_this();
    }
}
