#include "errors.hpp"

bool element::detail::register_errors()
{
    static bool registered_errors = false;
    if (registered_errors)
        return registered_errors;
    registered_errors = true;

    element::register_error<std::string, std::string>(
        error_message_code::failed_to_find_when_resolving_indexing_expr,
        "failed to find {} when indexing {}.\n",
        ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);

    element::register_error<std::string>(
        error_message_code::failed_to_find_when_resolving_identifier_expr,
        "failed to find {}.\n",
        ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);

    element::register_error<std::string, std::string>(
        error_message_code::recursion_detected,
        "failed at {}. recursion detected.\n",
        ELEMENT_ERROR_CIRCULAR_COMPILATION);

    return registered_errors;
}