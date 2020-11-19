#include "function_instance.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "instruction_tree/instructions.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/expressions/expression_chain.hpp"
#include "object_model/constraints/constraint.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/constraints/user_function_constraint.hpp"

using namespace element;

function_instance::function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info)
    : declarer(declarer)
    , inputs(declarer->get_inputs())
    , captures(std::move(captures))
    , our_constraint(std::make_unique<user_function_constraint>(declarer, false))
{
    this->source_info = std::move(source_info);
}

function_instance::function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info, std::vector<object_const_shared_ptr> args)
    : declarer(declarer)
    , inputs(inputs_except_provided)
    , inputs_except_provided(declarer->get_inputs().begin() + args.size(), declarer->get_inputs().end())
    , captures(std::move(captures))
    , provided_arguments(std::move(args))
    , our_constraint(std::make_unique<user_function_constraint>(declarer, true))
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
    return our_constraint->matches_constraint(context, constraint);
}

const constraint* function_instance::get_constraint() const
{
    return our_constraint.get();
}

bool has_defaults(const function_instance* instance)
{
    for (const auto& input : instance->declarer->inputs)
    {
        if (input.has_default())
            return true;
    }

    return false;
}

object_const_shared_ptr function_instance::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    compiled_args.insert(std::begin(compiled_args), std::begin(provided_arguments), std::end(provided_arguments));

    //todo: error checks

    const auto check_defaults = has_defaults(this) && !declarer->is_variadic();
    if (check_defaults)
    {
        const auto unfilled_args = declarer->inputs.size() - compiled_args.size() > 0;
        if (unfilled_args)
        {
            for (auto i = compiled_args.size(); i < declarer->inputs.size(); i++)
            {
                compiled_args.push_back(declarer->inputs[i].get_default()->compile(context, source_info));
            }
        }
    }

    if constexpr (should_log_compilation_step())
    {
        std::string input_string;
        for (std::size_t i = 0; i < declarer->inputs.size(); ++i)
        {
            input_string += fmt::format("{}{}{}",
                                        declarer->inputs[i].get_name(),
                                        declarer->inputs[i].has_annotation() ? ":" + declarer->inputs[i].get_annotation()->to_string() : "",
                                        i < compiled_args.size() ? " = " + compiled_args[i]->to_string() : "");

            if (i < static_cast<int>(declarer->inputs.size()) - 1)
                input_string += ", ";
        }

        context.get_logger()->log_step("{}({})\n", declarer->name.value, std::move(input_string));
        context.get_logger()->log_step("\\__\"{}\" @ {}:{}:{}\n", source_info.line_in_source->c_str(), source_info.filename, source_info.line, source_info.character_start);
        /*std::string indents(context.get_logger()->log_step_get_indent_level(), '\t');
        indents += std::string(source_info.character_start, ' ');
        std::string arrows(source_info.character_end - source_info.character_start, '^');
        context.get_logger()->log_step("    {}{}\n", indents, arrows);*/
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

    const auto us = shared_from_this();
    if (context.calls.recursive_calls(us) > 100)
        return context.calls.build_recursive_error(us, context, source_info);

    if constexpr (should_log_compilation_step())
        context.get_logger()->log_step_indent();

    context.calls.push(us, compiled_args);
    captures.push(declarer->our_scope.get(), &declarer->get_inputs(), compiled_args);

    std::swap(captures, context.captures);

    const auto visitor = [&context, &source_info](auto& body) {
        return body->compile(context, source_info);
    };

    auto element = std::visit(visitor, declarer->body);
    std::swap(captures, context.captures);

    captures.pop();
    context.calls.pop();
    if constexpr (should_log_compilation_step())
    {
        context.get_logger()->log_step_unindent();
        context.get_logger()->log_step("{} - returned '{}'\n",
                                       declarer->name.value, element->to_string());
    }

    if (!element)
        return std::make_shared<const error>(fmt::format("call to {} caused an internal error", declarer->name.value), ELEMENT_ERROR_UNKNOWN, source_info);

    //type check return
    //todo: nicer?
    const declaration* type = declarer->output.resolve_annotation(context);

    const constraint* constraint = nullptr;
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
        return call(context, {}, source_info);

    if (!variadic && provided_arguments.size() == declarer->inputs.size())
        return call(context, {}, source_info);

    return shared_from_this();
}

const std::vector<port>& function_instance::get_inputs() const
{
    return inputs;
}

bool function_instance::is_constant() const
{
    return true;
}

bool function_instance::valid_at_boundary(const compilation_context& context) const
{
    return declarer->valid_at_boundary(context);
}
