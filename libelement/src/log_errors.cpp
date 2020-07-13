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

        register_log_error<std::string>(log_error_message_code::parse_identifier_reserved,
            "identifier '{}' is reserved",
            ELEMENT_ERROR_RESERVED_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(log_error_message_code::parse_typename_not_identifier,
            "found '{}' when a type was expected.\nnote: a type must be an identifier or a chain of them, such as 'MyNamespace.MyStruct'.",
            ELEMENT_ERROR_INVALID_TYPENAME,
            ELEMENT_STAGE_PARSER);

        return true;
    }
}