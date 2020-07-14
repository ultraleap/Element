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

        register_log_error<std::string>(log_error_message_code::parse_port_failed,
            "found '{}' when a port was expected.\nnote: a port must be either an identifier or an underscore.",
            ELEMENT_ERROR_INVALID_PORT,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(log_error_message_code::parse_exprlist_empty,
            "expected to find something in the contents of the call to '{}', but nothing is contained within the parenthesis.\n"
            "note: perhaps you meant to call a function with no arguments, in which case you must omit the parenthesis.",
            ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL,
            ELEMENT_STAGE_PARSER);

        return true;
    }
}