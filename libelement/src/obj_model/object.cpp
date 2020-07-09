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
        return build_error(source_info, error_message_code::not_indexable);
    }

    std::shared_ptr<object> object::call(const compilation_context&, std::vector<std::shared_ptr<object>>, const source_information& source_info) const
    {
        return build_error(source_info, error_message_code::not_callable);
    }

    std::shared_ptr<object> object::compile(const compilation_context&, const source_information& source_info) const
    {
        return build_error(source_info, error_message_code::not_compilable);
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
}