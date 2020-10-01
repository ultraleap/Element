#include "function_instance.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "etree/expressions.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/constraints/constraint.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/compilation_context.hpp"

using namespace element;

function_instance::function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info)
    : declarer(declarer)
    , captures(std::move(captures))
{
    this->source_info = std::move(source_info);
}

function_instance::function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info, std::vector<object_const_shared_ptr> args)
    : declarer(declarer)
    , captures(std::move(captures))
    , provided_arguments(std::move(args))
{
    this->source_info = std::move(source_info);
}


std::string function_instance::to_string() const
{
    if (provided_arguments.empty())
        return declarer->name.value;

    std::string input_string;
    for (std::size_t i = 0; i < declarer->inputs.size(); ++i)
    {
        input_string += fmt::format("{}{}{}",
                                    declarer->inputs[i].get_name(),
                                    declarer->inputs[i].has_annotation() ? ":" + declarer->inputs[i].get_annotation()->to_string() : "",
                                    i < provided_arguments.size() ? " = " + provided_arguments[i]->to_string() : "");

        if (i < static_cast<int>(declarer->inputs.size()) - 1)
            input_string += ", ";
    }

    return fmt::format("{}({})", declarer->name.value, std::move(input_string));
}

bool function_instance::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    return declarer->get_constraint()->matches_constraint(context, constraint);
}

const constraint* function_instance::get_constraint() const
{
    return declarer->get_constraint();
}

object_const_shared_ptr function_instance::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    if constexpr (should_log_compilation_step())
        context.get_logger()->log_step("{} - calling {}function with {} arguments, plus {} already provided\n",
                                       declarer->name.value, declarer->is_variadic() ? "variadic " : "", compiled_args.size(), provided_arguments.size());

    compiled_args.insert(std::begin(compiled_args), std::begin(provided_arguments), std::end(provided_arguments));

    if constexpr (should_log_compilation_step())
    {
        std::string input_string;
        for (std::size_t i = 0; i < declarer->inputs.size(); ++i)
        {
            input_string += fmt::format("{}{}{}",
                declarer->inputs[i].get_name(),
                declarer->inputs[i].has_annotation() ? ":" + declarer->inputs[i].get_annotation()->to_string() : "",
                i < compiled_args.size()  ? " = " + compiled_args[i]->to_string() : ""); 

            if (i < static_cast<int>(declarer->inputs.size()) - 1)
                input_string += ", ";
        }
        context.get_logger()->log_step("\\___ {}({})\n", declarer->name.value, std::move(input_string));
    }

    const bool is_variadic = declarer->is_variadic();

    //element doesn't allow partial application generally
    //todo: more helpful information than the number of arguments expected
    if (compiled_args.size() < declarer->inputs.size())
        return build_error_and_log(context, source_info, error_message_code::not_enough_arguments,
                                   declarer->name.value, compiled_args.size(), declarer->inputs.size());

    //todo: more helpful information than the number of arguments expected
    if (!is_variadic && compiled_args.size() > declarer->inputs.size())
        return build_error_and_log(context, source_info, error_message_code::too_many_arguments,
                                   declarer->name.value, compiled_args.size(), declarer->inputs.size());

    //todo: check this works and is a useful error message (stolen from struct declaration call)
    if (!is_variadic && !valid_call(context, declarer, compiled_args))
        return build_error_for_invalid_call(context, declarer, compiled_args);

    if (context.calls.is_recursive(declarer))
        return context.calls.build_recursive_error(declarer, context, source_info);

    if constexpr (should_log_compilation_step())
    {
        context.get_logger()->log_step("{} - pushing to capture and call stack\n",
                                       declarer->name.value);
        context.get_logger()->log_step_indent();
    }

    context.calls.push(declarer, compiled_args);
    captures.push(declarer->our_scope.get(), &declarer->get_inputs(), compiled_args);

    std::swap(captures, context.captures);

    const auto visitor = [&context, &source_info](auto& body) {
        return body->compile(context, source_info);
    };

    auto element = std::visit(visitor, declarer->body);
    if constexpr (should_log_compilation_step())
    {
        context.get_logger()->log_step("{} - returned {}\n",
                                       declarer->name.value, element->to_string());
    }
    std::swap(captures, context.captures);

    captures.pop();
    context.calls.pop();
    if constexpr (should_log_compilation_step())
    {
        context.get_logger()->log_step_unindent();
        context.get_logger()->log_step("{} - popping from capture and call stack\n",
                                       declarer->name.value);
    }

    if (!element)
        return std::make_shared<const error>(fmt::format("call to {} caused an internal error", declarer->name.value), ELEMENT_ERROR_UNKNOWN, source_info);

    //type check return
    //todo: nicer?
    const constraint* constraint = nullptr;
    const declaration* type = nullptr;
    if (declarer->output)
        type = declarer->output->resolve_annotation(context);

    if (type)
        constraint = type->get_constraint();

    //todo: nicer error logs
    if (!element->matches_constraint(context, constraint))
        return std::make_shared<error>(
            fmt::format("the return of '{}' was '{}' which doesn't match the constraint '{}'",
                        declarer->name.value, element->typeof_info(), constraint ? type->name.value : "Any"),
            ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, source_info);

    return element;
}

object_const_shared_ptr function_instance::compile(const compilation_context& context,
                                                   const source_information& source_info) const
{
    const bool variadic = declarer->is_variadic();

    //variadic intrinsics (e.g. list) need at least one input, they aren't nullary
    if (variadic && !provided_arguments.empty())
    {
        if constexpr (should_log_compilation_step())
            context.get_logger()->log_step("{} - compiling variadic function with {} arguments\n",
                       declarer->name.value, provided_arguments.size());

        return call(context, {}, source_info);
    }

    if (!variadic && provided_arguments.size() == declarer->inputs.size())
    {
        if constexpr (should_log_compilation_step())
            context.get_logger()->log_step("{} - compiling function with {} arguments\n",
                declarer->name.value, provided_arguments.size());

        return call(context, {}, source_info);
    }

    /*if constexpr (should_log_compilation_step())
        context.get_logger()->log_step("{} - compiling function with {} arguments which is insufficient. {} arguments required\n",
                   declarer->name.value, provided_arguments.size(), declarer->inputs.size());*/

    return shared_from_this();
}

bool function_instance::is_constant() const
{
    return true;
}

bool function_instance::valid_at_boundary(const compilation_context& context) const
{
    return declarer->valid_at_boundary(context);
}
