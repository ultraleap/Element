#include "object.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "errors.hpp"
#include "scope.hpp"
#include "interpreter_internal.hpp"
#include "types.hpp"

namespace element
{
    identifier identifier::return_identifier{ "return" };

    compilation_context::compilation_context(const scope* const scope, element_interpreter_ctx* interpreter)
        : global_scope{ scope }
        , interpreter(interpreter)
    {

    }

    const element_log_ctx* compilation_context::get_logger() const
    {
        return interpreter->logger.get();
    }

    bool object::matches_constraint(const compilation_context& context, const constraint* constraint) const
    {
        if (!constraint || constraint == constraint::any.get())
            return true;

        return false;
    }

    std::shared_ptr<object> object::index(const compilation_context& context, const identifier&, const source_information& source_info) const
    {
        return build_error_and_log(context, source_info, error_message_code::not_indexable, typeof_info());
    }

    std::shared_ptr<object> object::call(const compilation_context& context, std::vector<std::shared_ptr<object>>, const source_information& source_info) const
    {
        return build_error_and_log(context, source_info, error_message_code::not_callable, typeof_info());
    }

    std::shared_ptr<object> object::compile(const compilation_context& context, const source_information& source_info) const
    {
        return build_error_and_log(context, source_info, error_message_code::not_compilable, typeof_info());
    }

    element_result error::get_result() const
    {
        return code;
    }

    const std::string& error::get_message() const
    {
        return message;
    }

    element_log_message error::get_log_message() const
    {
        element_log_message msg;
        msg.filename = source_info.filename;
        msg.line = source_info.line;
        msg.character = source_info.character_start;
        msg.message = message.c_str();
        msg.length = source_info.character_end - source_info.character_start;
        msg.message_code = code;
        msg.related_log_message = nullptr;
        msg.stage = ELEMENT_STAGE_COMPILER;
        msg.line_in_source = source_info.line_in_source ? source_info.line_in_source->c_str() : nullptr;
        return msg;
    }

    element_result error::log_once(const element_log_ctx* logger)
    {
        if (!logged)
        {
            logged = true;
            logger->log(get_log_message());
        }

        return code;
    }

    bool valid_call(const compilation_context& context, const declaration* declarer, const std::vector<std::shared_ptr<object>>& compiled_args)
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

            const auto type = input.resolve_annotation(context);
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

    std::shared_ptr<error> build_error_for_invalid_call(const compilation_context& context, const declaration* declarer, const std::vector<std::shared_ptr<object>>& compiled_args)
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

    std::shared_ptr<object> index_type(
        const declaration* type,
        std::shared_ptr<object> instance,
        const compilation_context& context,
        const identifier& name,
        const source_information& source_info)
    {
        auto func = std::dynamic_pointer_cast<function_declaration>(type->our_scope->find(name, false));

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

    call_stack::frame& call_stack::push(const declaration* function, std::vector<std::shared_ptr<object>> compiled_arguments)
    {
        return frames.emplace_back(frame{ function, std::move(compiled_arguments) });
    }

    void call_stack::pop()
    {
        frames.pop_back();
    }

    bool call_stack::is_recursive(const declaration* declaration) const
    {
        for (auto it = frames.rbegin(); it != frames.rend(); ++it)
        {
            if (it->function == declaration)
                return true;
        }

        return false;
    }

    std::shared_ptr<error> call_stack::build_recursive_error(
        const declaration* decl,
        const compilation_context& context,
        const source_information& source_info)
    {
        std::string trace;

        for (auto it = context.calls.frames.rbegin(); it < context.calls.frames.rend(); ++it)
        {
            auto& func = it->function;

            std::string params;
            for (unsigned i = 0; i < func->inputs.size(); ++i)
            {
                const auto& input = func->inputs[i];
                params += fmt::format("{}{} = {}",
                    input.get_name(),
                    input.has_annotation() ? ":" + input.get_annotation()->to_string() : "",
                    it->compiled_arguments[i]->typeof_info());

                if (i != func->inputs.size() - 1)
                    params += ", ";
            }

            trace += fmt::format("{}:{} at {}({})",
                func->source_info.filename,
                func->source_info.line,
                func->typeof_info(),
                params);

            if (func == decl)
                trace += " <-- here";

            if (it != context.calls.frames.rend() - 1)
                trace += "\n";
        }

        return build_error_and_log(context, source_info, error_message_code::recursion_detected, decl->typeof_info(), trace);
    }

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

    void capture_stack::push(const declaration* function, std::vector<std::shared_ptr<object>> compiled_arguments)
    {
        frames.emplace_back(frame{function, std::move(compiled_arguments)});
    }

    void capture_stack::pop()
    {
        frames.pop_back();
    }

    std::shared_ptr<object> capture_stack::find(const scope* s, const identifier& name)
    {
        while (s)
        {
            auto found = s->find(name, false);
            if (found)
                return found;

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
}