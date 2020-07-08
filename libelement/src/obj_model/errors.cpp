#include "errors.hpp"

bool element::detail::register_errors()
{
    static bool registered_errors = false;
    if (registered_errors)
        return registered_errors;
    registered_errors = true;

    element::register_error<>(
        error_message_code::not_indexable,
        "failed to index, it is not indexable.\n",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<>(
        error_message_code::not_callable,
        "failed to call, it is not callable.\n",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<>(
        error_message_code::not_compilable,
        "failed to compile, it is not compilable.\n",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<std::string>(
        error_message_code::not_indexable,
        "failed to index {}, it is not indexable.\n",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<std::string>(
        error_message_code::not_callable,
        "failed to call {}, it is not callable.\n",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<std::string>(
        error_message_code::not_compilable,
        "failed to compile {}, it is not compilable.\n",
        ELEMENT_ERROR_UNKNOWN);

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

    element::register_error<std::string, std::string>(
        error_message_code::instance_function_cannot_be_nullary,
        "error: '{}' was found when indexing '{}' but it is not an instance function as it has no parameters.\n",
        ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);

    element::register_error<std::string, std::string, std::string>(
        error_message_code::is_not_an_instance_function,
        "error: '{}' was found when indexing '{}' but it is not an instance function as it does not have an explicit type defined for its first parameter '{}'.\n",
        ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);

    element::register_error<std::string, std::string, std::string, std::string, std::string>(
        error_message_code::is_not_an_instance_function,
        "error: '{}' was found when indexing '{}' but it is not an instance function as the first parameter '{}' is of type '{}', when it needs to be '{}' to be considered an instance function.\n",
        ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);

    return registered_errors;
}