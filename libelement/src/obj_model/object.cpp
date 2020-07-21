#include "object.hpp"

//LIBS
#include <fmt/format.h>


#include "errors.hpp"
#include "scope.hpp"

namespace element
{
    identifier identifier::return_identifier{ "return" };

    compilation_context::compilation_context(const scope* const scope, element_interpreter_ctx* interpreter)
        : global_scope{ scope }
        , interpreter(interpreter)
    {

    }

    std::shared_ptr<object> object::index(const compilation_context&, const identifier&, const source_information& source_info) const
    {
        return build_error(source_info, error_message_code::not_indexable, typeof_info());
    }

    std::shared_ptr<object> object::call(const compilation_context&, std::vector<std::shared_ptr<object>>, const source_information& source_info) const
    {
        return build_error(source_info, error_message_code::not_callable, typeof_info());
    }

    std::shared_ptr<object> object::compile(const compilation_context&, const source_information& source_info) const
    {
        return build_error(source_info, error_message_code::not_compilable, typeof_info());
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

    bool valid_call(const declaration* declarer, const std::vector<std::shared_ptr<object>>& compiled_args)
    {
        if (compiled_args.size() != declarer->inputs.size())
            return false;

        //todo: check the types of each argument
        return true;
    }

    std::shared_ptr<error> error_for_invalid_call(const declaration* declarer, const std::vector<std::shared_ptr<object>>& compiled_args)
    {
        assert(!valid_call(declarer, compiled_args));

        if (compiled_args.size() != declarer->inputs.size())
        {
            std::string input_params;
            for (unsigned i = 0; i < declarer->inputs.size(); ++i)
            {
                const auto& input = declarer->inputs[i];
                input_params += fmt::format("({}) {}:{}", i, input.name.value, input.annotation->name.value);
                if (i != declarer->inputs.size() - 1)
                    input_params += ", ";
            }

            std::string given_params;
            for (unsigned i = 0; i < compiled_args.size(); ++i)
            {
                const auto& input = compiled_args[i];
                given_params += fmt::format("({}) {}", i, input->typeof_info());
                if (i != compiled_args.size() - 1)
                    given_params += ", ";
            }

            return build_error(declarer->source_info, error_message_code::argument_count_mismatch,
                declarer->location(), input_params, given_params);
        }

        assert(!"the call is valid");
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

        for (auto it = context.stack.frames.rbegin(); it < context.stack.frames.rend(); ++it)
        {
            auto& func = it->function;

            std::string params;
            for (unsigned i = 0; i < func->inputs.size(); ++i)
            {
                const auto& input = func->inputs[i];
                params += fmt::format("{}{} = {}",
                    input.name.value,
                    input.annotation ? ":" + input.annotation->name.value : "",
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

            if (it != context.stack.frames.rend() - 1)
                trace += "\n";
        }

        return build_error(source_info, error_message_code::recursion_detected, decl->typeof_info(), trace);
    }
}