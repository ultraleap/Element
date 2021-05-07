#pragma once

//STD
#include <memory>
#include <string>
#include <functional>
#include <map>

//LIBS
#include <fmt/format.h>

//SELF
#include "common_internal.hpp"
#include "ast/ast_internal.hpp"
#include "source_information.hpp"
#include "interpreter_internal.hpp"
#include "configuration.hpp"


namespace element
{
    enum class log_error_message_code
    {
        parse_identifier_failed,
        parse_identifier_reserved,
        parse_typename_not_identifier,
        parse_port_failed,
        parse_exprlist_empty,
        parse_exprlist_missing_closing_parenthesis,
        parse_call_invalid_expression,
        parse_expression_failed,
        parse_declaration_invalid_identifier,
        parse_declaration_missing_portlist_closing_parenthesis,
        parse_declaration_invalid_struct_return_type,
        parse_body_missing_semicolon,
        parse_body_missing_body_for_function,
        parse_body_missing_body,
        parse_function_missing_body,
        parse_struct_missing_identifier,
        parse_struct_nonintrinsic_missing_portlist,
        parse_struct_invalid_body,
        parse_constraint_invalid_identifier,
        parse_constraint_nonintrinsic_missing_portlist,
        parse_constraint_has_body,
        parse_constraint_invalid_body,
        intrinsic_not_implemented,
        intrinsic_type_mismatch,
        invalid_grammar_in_portlist,
        invalid_type_annotation,
        failed_to_build_declaration,
        failed_to_build_root,
        internal_compiler_error,
        missing_declaration_scope,
        expression_chain_cannot_be_empty,
        invalid_function_declaration,
        invalid_call_expression_placement,
        invalid_literal_expression_placement,
        empty_expression,
        function_missing_return,
        multiple_definition_with_parameter,
        default_argument_not_at_end,
        struct_portlist_cannot_contain_discards
    };
} // namespace element

//Note: Error C2888 FMT_COMPILE_STRING used by specializations can't be defined in a namespace, maybe a VS2017 bug (works with VS2019 and other OS's)
template <element::log_error_message_code>
struct log_error_message_info
{
    using tuple = void;
    static constexpr auto format = "";
    static constexpr element_result result_element = ELEMENT_ERROR_UNKNOWN;
    static constexpr element_stage stage = ELEMENT_STAGE_INVALID;
};

namespace element
{
    template <log_error_message_code code, typename... Args>
    std::string build_log_error_string(Args&&... args)
    {
        constexpr auto log_error_code_info_exists = !std::is_same_v<typename log_error_message_info<code>::tuple, void>;
        constexpr auto log_error_arguments_match = std::is_convertible_v<std::tuple<Args...>, typename log_error_message_info<code>::tuple>;
        static_assert(log_error_code_info_exists && log_error_arguments_match, "An error with that code that uses these types does not exist");
        
        return fmt::format(log_error_message_info<code>::format, std::forward<Args>(args)...);
    }

    template <log_error_message_code code, typename... Args>
    log_message build_log_error(const source_information& source_info, Args&&... args)
    {
        auto our_string = build_log_error_string<code>(std::forward<Args>(args)...);

        element_log_message msg;
        msg.line_in_source = source_info.line_in_source ? source_info.line_in_source->c_str() : nullptr;
        msg.character = source_info.character_start;
        msg.filename = source_info.filename;
        msg.message_code = log_error_message_info<code>::result_element;
        msg.line = source_info.line;
        msg.stage = log_error_message_info<code>::stage;
        msg.related_log_message = nullptr;                                    //todo: delete?
        msg.length = source_info.character_end - source_info.character_start; //todo: UTF8 concerns?
        msg.message = nullptr;
        msg.message_length = 0;

        return {std::move(msg), std::move(our_string)};
    }

    template <log_error_message_code code, typename... Args>
    element_result log_error(const element_log_ctx* logger, Args&&... args)
    {
        auto error = build_log_error<code>(std::forward<Args>(args)...);
        if (logger)
            logger->log(error);
        return error.result;
    }

    template <log_error_message_code code, typename... Args>
    element_result log_error(const element_interpreter_ctx* context, Args&&... args)
    {
        return log_error<code>(context->logger.get(), std::forward<Args>(args)...);
    }

    inline source_information build_source_info(const source_context* context, const element_token* token, int extra_length)
    {
        if (!context || !token)
            return {};

        const auto& file_info = context->file_info.at(token->source_name);
        const std::string* filename = file_info.file_name.get();
        const int actual_line = token->line - 1;
        const std::string* line_in_source = actual_line >= 0 ? file_info.source_lines[actual_line].get() : nullptr;

        return {
            token->line,
            token->character,
            token->character + token->tok_len + extra_length,
            line_in_source,
            filename->data()
        };
    }

    inline source_information build_source_info(const source_context* context, const element_token* token)
    {
        return build_source_info(context, token, 0);
    }

    template <log_error_message_code code, typename... Args>
    log_message build_log_error(const source_context* context, const element_token* token, Args... args)
    {
        return build_log_error<code>(build_source_info(context, token), std::forward<Args>(args)...);
    }

    template <log_error_message_code code, typename... Args>
    log_message build_log_error(const source_context* context, const element_ast* ast, Args... args)
    {
        if (ast && ast->nearest_token)
        {
            log_message msg = build_log_error<code>(context, ast->nearest_token, std::forward<Args>(args)...);

            const auto starts_with_prelude = std::string(msg.get_log_message().filename).rfind("Prelude/", 0) == 0;
            const auto log_ast = starts_with_prelude
                                     ? flag_set(logging_bitmask, log_flags::output_prelude) && flag_set(logging_bitmask, log_flags::output_ast)
                                     : flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

            if (ast && log_ast)
                msg.append_text("\n---\nAST\n---\n" + ast_to_string(ast->get_root(), 0, ast));

            return msg;
        }

        return build_log_error<code>(source_information{}, std::forward<Args>(args)...);
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

#define MAKE_LOG_ERROR_MESSAGE_INFO(c, res, stage_, s, ...) \
template<> \
struct log_error_message_info<c> \
{ \
    using tuple = std::tuple<__VA_ARGS__>; \
    static constexpr auto format = FMT_STRING(s); \
    static constexpr element_result result_element = res; \
    static constexpr element_stage stage = stage_; \
    static inline const auto format_check = []() -> std::string { \
        if constexpr(!std::is_same_v<tuple, std::tuple<>> && !std::is_same_v<tuple, void>) \
        { \
            return std::apply(element::build_log_error_string<c, __VA_ARGS__>, tuple{}); \
        } \
        return ""; \
    }(); \
};

MAKE_LOG_ERROR_MESSAGE_INFO(
    element::log_error_message_code::parse_identifier_failed,
    element_result::ELEMENT_ERROR_INVALID_IDENTIFIER,
    element_stage::ELEMENT_STAGE_PARSER,
    "invalid identifier '{}'",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_identifier_reserved,
    element_result::ELEMENT_ERROR_RESERVED_IDENTIFIER,
    element_stage::ELEMENT_STAGE_PARSER,
    "identifier '{}' is reserved",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_typename_not_identifier,
    element_result::ELEMENT_ERROR_INVALID_TYPENAME,
    element_stage::ELEMENT_STAGE_PARSER,
    "found '{}' when a type was expected."
    "\nnote: a type must be an identifier or a chain of them, such as 'MyNamespace.MyStruct'.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_port_failed,
    element_result::ELEMENT_ERROR_INVALID_PORT,
    element_stage::ELEMENT_STAGE_PARSER,
    "found '{}' when a port was expected."
    "\nnote: a port must be either an identifier or an underscore.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_exprlist_empty,
    element_result::ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find something in the contents of the call to '{}', but nothing is contained within the parenthesis."
    "\nnote: perhaps you meant to call a function with no arguments, in which case you must omit the parenthesis.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_exprlist_missing_closing_parenthesis,
    element_result::ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find a ')' at the end of the call to '{}', but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_call_invalid_expression,
    element_result::ELEMENT_ERROR_INVALID_CONTENTS_FOR_CALL,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find an identifier or number in the contents of the call to '{}', but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_expression_failed,
    element_result::ELEMENT_ERROR_INVALID_EXPRESSION,
    element_stage::ELEMENT_STAGE_PARSER,
    "failed to parse the expression '{}'."
    "\nnote: it must start with an identifier, an underscore, or a number.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_declaration_invalid_identifier,
    element_result::ELEMENT_ERROR_INVALID_IDENTIFIER,
    element_stage::ELEMENT_STAGE_PARSER,
    "failed to parse the expression '{}'."
    "\nfound '{}' when parsing a declaration, expected a valid identifier.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_declaration_missing_portlist_closing_parenthesis,
    element_result::ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST,
    element_stage::ELEMENT_STAGE_PARSER,
    "found '{}' at the end of the declaration for '{}' with a portlist. expected ')'",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_declaration_invalid_struct_return_type,
    element_result::ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE,
    element_stage::ELEMENT_STAGE_PARSER,
    "the struct '{}' has a return type declared."
    "\nnote: structs can't have return types",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_body_missing_semicolon,
    element_result::ELEMENT_ERROR_MISSING_SEMICOLON,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find a ';' at the end of the expression for '{}', but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_body_missing_body_for_function,
    element_result::ELEMENT_ERROR_MISSING_FUNCTION_BODY,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find a body for the non-intrinsic function '{}' but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_body_missing_body,
    element_result::ELEMENT_ERROR_MISSING_BODY,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find a body for '{}' but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_function_missing_body,
    element_result::ELEMENT_ERROR_MISSING_FUNCTION_BODY,
    element_stage::ELEMENT_STAGE_PARSER,
    "non-intrinsic function '{}' is missing a body.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_struct_missing_identifier,
    element_result::ELEMENT_ERROR_INVALID_IDENTIFIER,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find an identifier for a struct, but found '{}' instead.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_struct_nonintrinsic_missing_portlist,
    element_result::ELEMENT_ERROR_MISSING_PORTS,
    element_stage::ELEMENT_STAGE_PARSER,
    "portlist for struct '{}' is required as it is not intrinsic.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_struct_invalid_body,
    element_result::ELEMENT_ERROR_STRUCT_INVALID_BODY,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find '{{' or '=' before the body of the struct '{}' but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_constraint_invalid_identifier,
    element_result::ELEMENT_ERROR_INVALID_IDENTIFIER,
    element_stage::ELEMENT_STAGE_PARSER,
    "found '{}' when parsing a constraint, expected a valid identifier.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_constraint_nonintrinsic_missing_portlist,
    element_result::ELEMENT_ERROR_MISSING_PORTS,
    element_stage::ELEMENT_STAGE_PARSER,
    "portlist for constraint '{}' is required as it is not intrinsic.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_constraint_has_body,
    element_result::ELEMENT_ERROR_CONSTRAINT_HAS_BODY,
    element_stage::ELEMENT_STAGE_PARSER,
    "a body was found for constraint '{}', but constraints cannot have bodies.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::parse_constraint_invalid_body,
    element_result::ELEMENT_ERROR_CONSTRAINT_INVALID_BODY,
    element_stage::ELEMENT_STAGE_PARSER,
    "expected to find '{{' or '=' before the body of the constraint '{}' but found '{}' instead.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::intrinsic_not_implemented,
    element_result::ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED,
    element_stage::ELEMENT_STAGE_PARSER,
    "intrinsic {} not implemented.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::invalid_type_annotation,
    element_result::ELEMENT_ERROR_TYPE_ERROR,
    element_stage::ELEMENT_STAGE_PARSER,
    "type annotation was invalid."
    "\n{}",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::intrinsic_type_mismatch,
    element_result::ELEMENT_ERROR_TYPE_ERROR,
    element_stage::ELEMENT_STAGE_PARSER,
    "intrinsic '{}' type mismatch",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::invalid_grammar_in_portlist,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "portlist for '{}' contains invalid grammar",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::failed_to_build_declaration,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "failed to build declaration '{}'",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::failed_to_build_root,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "failed to build global scope for '{}'",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::internal_compiler_error,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "internal compiler error",
    NOARG
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::missing_declaration_scope,
    element_result::ELEMENT_ERROR_INVALID_EXPRESSION,
    element_stage::ELEMENT_STAGE_PARSER,
    "'{}' must have a scope",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::expression_chain_cannot_be_empty,
    element_result::ELEMENT_ERROR_INVALID_EXPRESSION,
    element_stage::ELEMENT_STAGE_PARSER,
    "the expression chain belonging to function '{}' cannot be empty",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::invalid_function_declaration,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "invalid function declaration '{}'",
    std::string
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::invalid_call_expression_placement,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "call expression cannot appear as the first item in an expression chain",
    NOARG
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::invalid_literal_expression_placement,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "literal expression cannot appear as anything other than the first item in an expression chain",
    NOARG
);

//todo: element_result
MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::empty_expression,
    element_result::ELEMENT_ERROR_UNKNOWN,
    element_stage::ELEMENT_STAGE_PARSER,
    "no expression found",
    NOARG
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::function_missing_return,
    element_result::ELEMENT_ERROR_MISSING_FUNCTION_RETURN,
    element_stage::ELEMENT_STAGE_PARSER,
    "non-intrinsic scope-bodied function '{}' is missing a return declaration.",
    std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::multiple_definition_with_parameter,
    element_result::ELEMENT_ERROR_MULTIPLE_DEFINITIONS,
    element_stage::ELEMENT_STAGE_PARSER,
    "declaration '{}' within function '{}' has the same name as a parameter.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::default_argument_not_at_end,
    element_result::ELEMENT_ERROR_DEFAULT_ARGUMENT_NOT_AT_END,
    element_stage::ELEMENT_STAGE_PARSER,
    "argument '{}' for function '{}' has a default, but parameters with defaults must be defined at the end of the portlist.",
    std::string, std::string
);

MAKE_LOG_ERROR_MESSAGE_INFO(
   element::log_error_message_code::struct_portlist_cannot_contain_discards,
    element_result::ELEMENT_ERROR_STRUCT_PORTLIST_CONTAINS_DISCARDS,
    element_stage::ELEMENT_STAGE_PARSER,
    "the port for the struct declaration '{}' contains a discarded('_') parameter, which isn't allowed here",
    std::string
);