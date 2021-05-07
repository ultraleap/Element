#pragma once

//STD
#include <memory>
#include <string>

//LIBS
#include <fmt/format.h>

//SELF
#include "object_internal.hpp"
#include "error.hpp"
#include "compilation_context.hpp"

namespace element
{
    enum class error_message_code
    {
        not_indexable,
        not_callable,
        not_compilable,
        failed_to_find_when_resolving_indexing_expr,
        failed_to_find_when_resolving_identifier_expr,
        recursion_detected,
        instance_function_cannot_be_nullary,
        is_not_an_instance_function_any_type,
        is_not_an_instance_function_wrong_type,
        argument_count_mismatch,
        failed_to_find,
        intrinsic_not_implemented,
        not_enough_arguments,
        too_many_arguments,
        invalid_errorless_call,
    };
} // namespace element

//Note: Error C2888 FMT_COMPILE_STRING used by specializations can't be defined in a namespace, maybe a VS2017 bug (works with VS2019 and other OS's)
template <element::error_message_code>
struct error_message_info
{
    using tuple = void;
    static constexpr auto format = "";
};
    
namespace element
{
    template <error_message_code code, typename... Args>
    std::string build_error_string(Args&&... args)
    {
        constexpr auto log_error_code_info_exists = !std::is_same_v<typename error_message_info<code>::tuple, void>;
        constexpr auto log_error_arguments_match = std::is_convertible_v<std::tuple<Args...>, typename error_message_info<code>::tuple>;
        static_assert(log_error_code_info_exists && log_error_arguments_match, "An error with that code that uses these types does not exist");
        
        return fmt::format(error_message_info<code>::format, std::forward<Args>(args)...);
    }

    template <error_message_code code, typename... Args>
    std::shared_ptr<element::error> build_error(const source_information& source_info, Args&&... args)
    {
        return std::make_shared<error>(build_error_string<code>(std::forward<Args>(args)...), error_message_info<code>::result_element, source_info);
    }

    template <error_message_code code, typename... Args>
    std::shared_ptr<error> build_error_and_log(const compilation_context& context, Args&&... args)
    {
        auto error = build_error<code>(std::forward<Args>(args)...);
        error->log_once(context.get_logger());
        return error;
    }
} // namespace element

#ifndef NOARG
    #define NOARG
#endif

/**
 * This macro, which should only be used in this header file, is simply a convenient way of doing a struct specialisation
 * This struct stores all the compile-time information that is later required when constructing the message, to ensure at compile-time that it is valid, rather than later throw an exception
 * format_check will attempt to do a test-run of formatting the error using default-constructed parameters, catching issues with it at compile-time even if the message itself is never used.
 *     note that it may need to be modified in the future to handle arguments that can't be default-constructed, in which case the if-constexpr statement should be extended
 */

#define MAKE_ERROR_MESSAGE_INFO(c, res, s, ...) \
template<> \
struct error_message_info<c> \
{ \
    using tuple = std::tuple<__VA_ARGS__>; \
    static constexpr auto format = FMT_STRING(s); \
    static constexpr element_result result_element = res; \
    static inline const auto format_check = []() -> std::string { \
        if constexpr(!std::is_same_v<tuple, std::tuple<>> && !std::is_same_v<tuple, void>) \
        { \
            return std::apply(element::build_error_string<c, __VA_ARGS__>, tuple{}); \
        } \
        return ""; \
    }(); \
};

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::not_indexable,
    element_result::ELEMENT_ERROR_NOT_INDEXABLE,
    "failed to index '{}', it is not indexable.",
    std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::not_callable,
    element_result::ELEMENT_ERROR_INVALID_CALL_NONFUNCTION,
    "failed to call '{}', it is not callable.",
    std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::not_compilable,
    element_result::ELEMENT_ERROR_UNKNOWN,
    "failed to compile '{}', it is not compilable.",
    std::string
);

//todo: not currently used, try and be more specific if possible
MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::failed_to_find,
    element_result::ELEMENT_ERROR_IDENTIFIER_NOT_FOUND,
    "failed to find '{}'.",
    std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::failed_to_find_when_resolving_indexing_expr,
    element_result::ELEMENT_ERROR_IDENTIFIER_NOT_FOUND,
    "failed to find '{}' when indexing '{}'.",
    std::string, std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::failed_to_find_when_resolving_identifier_expr,
    element_result::ELEMENT_ERROR_IDENTIFIER_NOT_FOUND,
    "failed to find '{}'.",
    std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::recursion_detected,
    element_result::ELEMENT_ERROR_CIRCULAR_COMPILATION,
    "failed at '{}'. recursion detected."
    "\n{}",
    std::string, std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::instance_function_cannot_be_nullary,
    element_result::ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION,
    "'{}' was found when indexing '{}' but it is not an instance function."
    "\nnote: the function has no parameters.",
    std::string, std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::is_not_an_instance_function_any_type,
    element_result::ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION,
    "'{}' was found when indexing '{}' but it is not an instance function."
    "\nnote: the function does not have an explicit type defined for its first parameter '{}'.",
    std::string, std::string, std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::is_not_an_instance_function_wrong_type,
    element_result::ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION,
    "'{}' was found when indexing '{}' but it is not an instance function."
    "\nnote: the first parameter '{}' is of type '{}'."
    "\nThe first parameter needs to be of type '{}' for it to be considered an instance function.",
    std::string, std::string, std::string, std::string, std::string
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::argument_count_mismatch,
    element_result::ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH,
    "attempted to construct an instance of '{}' with the incorrect number of arguments."
    "\nnote: {} arguments are required, but {} were passed."
    "\nThe parameters are '{}'."
    "\nThe arguments are '{}'.",
    std::string, std::size_t, std::size_t, std::string, std::string
);

//expand on this
MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::intrinsic_not_implemented,
    element_result::ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED,
    "intrinsic not implemented.",
    NOARG
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::not_enough_arguments,
    element_result::ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH,
    "the call to '{0}' does not contain enough arguments."
    "\nnote: '{0}' was called with {1} arguments, when {2} arguments were expected.",
    std::string, size_t, size_t
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::too_many_arguments,
    element_result::ELEMENT_ERROR_ARGUMENT_COUNT_MISMATCH,
    "the call to '{0}' contains too many arguments."
    "\nnote: '{0}' was called with {1} arguments, when {2} arguments were expected.",
    std::string, size_t, size_t
);

MAKE_ERROR_MESSAGE_INFO(
    element::error_message_code::invalid_errorless_call,
    element_result::ELEMENT_ERROR_UNKNOWN,
    "the call to '{}' caused an internal compiler error.",
    std::string
);