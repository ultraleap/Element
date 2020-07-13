#include "log_errors.hpp"

namespace element::detail
{
    bool register_log_errors()
    {
        static bool registered_errors = false;
        if (registered_errors)
            return registered_errors;
        registered_errors = true;

        register_log_error<std::string>(log_error_message_code::parse_identifier_failed, 
            "invalid identifier '{}'",
            ELEMENT_ERROR_INVALID_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        return true;
    }
}