#include "object.hpp"

//LIBS
#include <fmt/format.h>

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
        return std::make_shared<error>(
            fmt::format("failed to index {}. it is not indexable.\n"),
            ELEMENT_ERROR_UNKNOWN,
            nullptr);
    }

    std::shared_ptr<object> object::call(const compilation_context&, std::vector<std::shared_ptr<object>>) const
    {
        return std::make_shared<error>(
            fmt::format("failed to call {}. it is not callable.\n"),
            ELEMENT_ERROR_UNKNOWN,
            nullptr);
    }

    std::shared_ptr<object> object::compile(const compilation_context&) const
    {
        return std::make_shared<error>(
            fmt::format("failed to compile {}. it is not compilable.\n"),
            ELEMENT_ERROR_UNKNOWN,
            nullptr);
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