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
}