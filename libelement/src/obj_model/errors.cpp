#include "errors.hpp"

bool element::detail::register_errors()
{
    static bool registered_errors = false;
    if (registered_errors)
        return registered_errors;
    registered_errors = true;

    element::register_error<std::string>(
        error_message_code::not_indexable,
        "failed to index '{}', it is not indexable.",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<std::string>(
        error_message_code::not_callable,
        "failed to call '{}', it is not callable.",
        ELEMENT_ERROR_UNKNOWN);

    element::register_error<std::string>(
        error_message_code::not_compilable,
        "failed to compile '{}', it is not compilable.",
        ELEMENT_ERROR_UNKNOWN);

    //todo: not currently used, try and be more specific if possible
    element::register_error<std::string>(
        error_message_code::failed_to_find,
        "failed to find '{}'.",
        ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);

    element::register_error<std::string, std::string>(
        error_message_code::failed_to_find_when_resolving_indexing_expr,
        "failed to find '{}' when indexing '{}'.",
        ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);

    element::register_error<std::string>(
        error_message_code::failed_to_find_when_resolving_identifier_expr,
        "failed to find '{}'.",
        ELEMENT_ERROR_IDENTIFIER_NOT_FOUND);

    element::register_error<std::string, std::string>(
        error_message_code::recursion_detected,
        "failed at '{}'. recursion detected.\n{}",
        ELEMENT_ERROR_CIRCULAR_COMPILATION);

    element::register_error<std::string, std::string>(
        error_message_code::instance_function_cannot_be_nullary,
        "'{}' was found when indexing '{}' but it is not an instance function.\nnote: the function has no parameters.",
        ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);

    element::register_error<std::string, std::string, std::string>(
        error_message_code::is_not_an_instance_function,
        "'{}' was found when indexing '{}' but it is not an instance function.\nnote: the function does not have an explicit type defined for its first parameter '{}'.",
        ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);

    element::register_error<std::string, std::string, std::string, std::string, std::string>(
        error_message_code::is_not_an_instance_function,
        "'{}' was found when indexing '{}' but it is not an instance function.\nnote: the first parameter '{}' is of type '{}'. the first parameter needs to be '{}' for this to be considered an instance function.",
        ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION);

    element::register_error<std::string, std::string, std::string>(
        error_message_code::argument_count_mismatch,
        "attempted to construct an instance of '{}' which requires the parameters '{}', but the parameters of type '{}' were used instead.",
        ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH);

    return registered_errors;
}