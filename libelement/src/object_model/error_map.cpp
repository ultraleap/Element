#include "error_map.hpp"

bool element::detail::register_errors()
{
    static bool registered_errors = false;
    if (registered_errors)
        return registered_errors;
    registered_errors = true;

    element::register_error<std::string>(
        error_message_code::not_indexable,
        "failed to index '{}', it is not indexable.",
        ELEMENT_ERROR_NOT_INDEXABLE);

    element::register_error<std::string>(
        error_message_code::not_callable,
        "failed to call '{}', it is not callable.",
        ELEMENT_ERROR_INVALID_CALL_NONFUNCTION);

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

    element::register_error<std::string, std::size_t, std::size_t, std::string, std::string>(
        error_message_code::argument_count_mismatch,
        "attempted to construct an instance of '{}' with the incorrect number of arguments."
        "\nnote: {} arguments are required, but {} were passed."
        "\nThe parameters are '{}'."
        "\nThe arguments are '{}'.",
        ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH);

    //expand on this
    element::register_error<>(
        error_message_code::intrinsic_not_implemented,
        "intrinsic not implemented.",
        ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED);

    element::register_error<std::string, size_t, size_t>(
        error_message_code::not_enough_arguments,
        "the call to '{0}' does not contain enough arguments.\nnote: '{0}' was called with {1} arguments, when {2} arguments were expected.",
        ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH);

    element::register_error<std::string, size_t, size_t>(
        error_message_code::too_many_arguments,
        "the call to '{0}' contains too many arguments.\nnote: '{0}' was called with {1} arguments, when {2} arguments were expected.",
        ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH);

    element::register_error<>(
        error_message_code::invalid_errorless_call,
        "the call to '{}' caused an internal compiler error.",
        ELEMENT_ERROR_UNKNOWN);

    return registered_errors;
}