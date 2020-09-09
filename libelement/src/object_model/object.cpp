#include "object.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "declarations/function_declaration.hpp"
#include "constraints/constraint.hpp"
#include "error.hpp"
#include "error_map.hpp"
#include "compilation_context.hpp"
#include "intrinsics/intrinsic.hpp"

namespace element
{
    bool object::is_constant() const
    {
        return false;
    }

    bool object::matches_constraint(const compilation_context& context, const constraint* constraint) const
    {
        if (!constraint || constraint == constraint::any.get())
            return true;

        return false;
    }

    object_const_shared_ptr object::index(const compilation_context& context, const identifier&,
                                                const source_information& source_info) const
    {
        return build_error_and_log(context, source_info, error_message_code::not_indexable, typeof_info());
    }

    object_const_shared_ptr object::call(const compilation_context& context, std::vector<object_const_shared_ptr>,
                                               const source_information& source_info) const
    {
        return build_error_and_log(context, source_info, error_message_code::not_callable, typeof_info());
    }

    object_const_shared_ptr object::compile(const compilation_context& context,
                                                  const source_information& source_info) const
    {
        return build_error_and_log(context, source_info, error_message_code::not_compilable, typeof_info());
    }

    bool valid_call(const compilation_context& context, const declaration* declarer, const std::vector<object_const_shared_ptr>&
                    compiled_args)
    {
        if (compiled_args.size() != declarer->inputs.size())
            return false;

        for (unsigned int i = 0; i < compiled_args.size(); ++i)
        {
            const auto& arg = compiled_args[i];
            const auto& input = declarer->inputs[i];

            //no annotation always matches
            if (!input.has_annotation())
                return true;

            const auto* const type = input.resolve_annotation(context);
            if (!type)
            {
                assert(!"failed to resolve annotation, couldn't be found");
                return false;
            }

            if (!arg->matches_constraint(context, type->get_constraint()))
                return false;
        }

        return true;
    }

    std::shared_ptr<const error> build_error_for_invalid_call(const compilation_context& context, const declaration* declarer, const std::vector<object_const_shared_ptr>& compiled_args)
    {
        assert(!valid_call(context, declarer, compiled_args));

        if (compiled_args.size() != declarer->inputs.size())
        {
            std::string input_params;
            for (unsigned i = 0; i < declarer->inputs.size(); ++i)
            {
                const auto& input = declarer->inputs[i];
                input_params += fmt::format("({}) {}{}", i, input.get_name(), input.has_annotation() ? ":" + input.get_annotation()->to_string() : "");
                if (i != declarer->inputs.size() - 1)
                    input_params += ", ";
            }

            std::string given_params;
            for (unsigned i = 0; i < compiled_args.size(); ++i)
            {
                const auto& input = compiled_args[i];
                given_params += fmt::format("({}) _:{}", i, input->typeof_info());
                if (i != compiled_args.size() - 1)
                    given_params += ", ";
            }

            return build_error(declarer->source_info, error_message_code::argument_count_mismatch,
                declarer->location(), input_params, given_params);
        }

        //todo: proper logging
        return std::make_shared<error>("constraint not satisfied", ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED, declarer->source_info);
    }

    object_const_shared_ptr index_type(const declaration* type,
                                       object_const_shared_ptr instance,
                                       const compilation_context& context,
                                       const identifier& name,
                                       const source_information& source_info)
    {
        const auto func = dynamic_cast<const function_declaration*>(type->our_scope->find(name, false));

        //todo: not exactly working type checking, good enough for now though
        const bool has_inputs = func && func->has_inputs();
        const bool has_type = has_inputs && func->inputs[0].get_annotation();
        const bool types_match = has_type && func->inputs[0].get_annotation()->to_string() == type->name.value;

        //call as instance function, filling in first argument
        if (types_match)
            return func->call(context, { std::move(instance) }, source_info);

        //todo: instead of relying on generic error handling for nullptr, build a specific error
        if (!func)
            return nullptr;

        if (!has_inputs)
            return build_error_and_log(context, source_info, error_message_code::instance_function_cannot_be_nullary,
                func->typeof_info(), instance->typeof_info());

        if (!has_type)
            return build_error_and_log(context, source_info, error_message_code::is_not_an_instance_function,
                func->typeof_info(), instance->typeof_info(), func->inputs[0].get_name());

        if (!types_match)
            return build_error_and_log(context, source_info, error_message_code::is_not_an_instance_function,
                func->typeof_info(), instance->typeof_info(),
                func->inputs[0].get_name(), func->inputs[0].get_annotation()->to_string(), type->name.value);

        //did we miss an error that we need to handle?
        assert(false);
        return nullptr;
    }


    object_const_shared_ptr compile_placeholder_expression(const compilation_context& context,
        const object& object,
        const std::vector<port>& inputs,
        element_result& result,
        const source_information& source_info,
        const int placeholder_offset)
    {
        auto [placeholder, size] = generate_placeholder_inputs(context, inputs, result, placeholder_offset);
        if (result != ELEMENT_OK)
        {
            result = ELEMENT_ERROR_UNKNOWN;
            return nullptr;
        }

        context.boundaries.push_back({ size });
        auto compiled = object.call(context, std::move(placeholder), source_info);
        context.boundaries.pop_back();

        if (!compiled)
        {
            result = ELEMENT_ERROR_UNKNOWN;
            return nullptr;
        }

        const auto err = std::dynamic_pointer_cast<const element::error>(compiled);
        if (err)
        {
            result = err->log_once(context.interpreter->logger.get());
            return err;
        }

        return compiled;
    }

}