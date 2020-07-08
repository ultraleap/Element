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

    std::shared_ptr<object> object::index(const compilation_context&, const identifier&) const
    {
        return build_error<>(error_message_code::not_indexable);
    }

    std::shared_ptr<object> object::call(const compilation_context&, std::vector<std::shared_ptr<object>>) const
    {
        return build_error<>(error_message_code::not_callable);
    }

    std::shared_ptr<object> object::compile(const compilation_context&) const
    {
        return build_error<>(error_message_code::not_compilable);
    }

    element_result error::get_result() const
    {
        return code;
    }

    const std::string& error::get_message() const
    {
        return message;
    }

    const declaration* error::get_declaration() const
    {
        return location;
    }

    const std::shared_ptr<error>& error::get_wrapped_error() const
    {
        return err;
    }
}