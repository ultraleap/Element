#include "log_errors.hpp"

#include "obj_model/errors.hpp"

namespace element::detail
{
    bool register_log_errors()
    {
        static bool registered_errors = false;
        if (registered_errors)
            return registered_errors;
        registered_errors = true;

        register_log_error<std::string>(
            log_error_message_code::parse_identifier_failed, 
            "invalid identifier '{}'",
            ELEMENT_ERROR_INVALID_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_identifier_reserved,
            "identifier '{}' is reserved",
            ELEMENT_ERROR_RESERVED_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_typename_not_identifier,
            "found '{}' when a type was expected.\nnote: a type must be an identifier or a chain of them, such as 'MyNamespace.MyStruct'.",
            ELEMENT_ERROR_INVALID_TYPENAME,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_port_failed,
            "found '{}' when a port was expected.\nnote: a port must be either an identifier or an underscore.",
            ELEMENT_ERROR_INVALID_PORT,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_exprlist_empty,
            "expected to find something in the contents of the call to '{}', but nothing is contained within the parenthesis.\n"
            "note: perhaps you meant to call a function with no arguments, in which case you must omit the parenthesis.",
            ELEMENT_ERROR_MISSING_CONTENTS_FOR_CALL,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_exprlist_missing_closing_parenthesis,
            "expected to find a ')' at the end of the call to '{}', but found '{}' instead.",
            ELEMENT_ERROR_MISSING_PARENTHESIS_FOR_CALL,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_call_invalid_expression,
            "expected to find an identifier or number in the contents of the call to '{}', but found '{}' instead.",
            ELEMENT_ERROR_INVALID_CONTENTS_FOR_CALL,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_expression_failed,
            "failed to parse the expression '{}'.\nnote: it must start with an identifier, an underscore, or a number.",
            ELEMENT_ERROR_INVALID_EXPRESSION,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_declaration_invalid_identifier,
            "found '{}' when parsing a declaration, expected a valid identifier.",
            ELEMENT_ERROR_INVALID_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_declaration_missing_portlist_closing_parenthesis,
            "found '{}' at the end of the declaration for '{}' with a portlist. expected ')'",
            ELEMENT_ERROR_MISSING_CLOSING_PARENTHESIS_FOR_PORTLIST,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_declaration_invalid_struct_return_type,
            "the struct '{}' has a return type declared.\nnote: structs can't have return types",
            ELEMENT_ERROR_STRUCT_CANNOT_HAVE_RETURN_TYPE,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_body_missing_semicolon,
            "expected to find a ';' at the end of the expression for '{}', but found '{}' instead.",
            ELEMENT_ERROR_MISSING_SEMICOLON,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_body_missing_body_for_function,
            "expected to find a body for the non-intrinsic function '{}' but found '{}' instead.",
            ELEMENT_ERROR_MISSING_FUNCTION_BODY,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_body_missing_body,
            "expected to find a body for '{}' but found '{}' instead.",
            ELEMENT_ERROR_MISSING_BODY,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_function_missing_body,
            "non-intrinsic function '{}' is missing a body.",
            ELEMENT_ERROR_MISSING_FUNCTION_BODY,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_struct_missing_identifier,
            "expected to find an identifier for a struct, but found '{}' instead.",
            ELEMENT_ERROR_INVALID_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_struct_nonintrinsic_missing_portlist,
            "portlist for struct '{}' is required as it is not intrinsic.",
            ELEMENT_ERROR_MISSING_PORTS,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_struct_invalid_body,
            "expected to find '{' or '=' before the body of the struct '{}' but found '{}' instead.",
            ELEMENT_ERROR_STRUCT_INVALID_BODY,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_constraint_invalid_identifier,
            "found '{}' when parsing a constraint, expected a valid identifier.",
            ELEMENT_ERROR_INVALID_IDENTIFIER,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_constraint_nonintrinsic_missing_portlist,
            "portlist for constraint '{}' is required as it is not intrinsic.",
            ELEMENT_ERROR_MISSING_PORTS,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::parse_constraint_has_body,
            "a body was found for constraint '{}', but constraints cannot have bodies.",
            ELEMENT_ERROR_CONSTRAINT_HAS_BODY,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string, std::string>(
            log_error_message_code::parse_constraint_invalid_body,
            "expected to find '{' or '=' before the body of the constraint '{}' but found '{}' instead.",
            ELEMENT_ERROR_CONSTRAINT_INVALID_BODY,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::intrinsic_not_implemented,
            "intrinsic {} not implemented.",
            ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED,
            ELEMENT_STAGE_PARSER);

        register_log_error<std::string>(
            log_error_message_code::intrinsic_type_mismatch,
            "intrinsic {} type mismatch",
            ELEMENT_ERROR_TYPE_ERROR,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<std::string>(
            log_error_message_code::invalid_grammar_in_portlist,
            "portlist for '{}' contains invalid grammar",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<std::string>(
            log_error_message_code::failed_to_build_declaration,
            "failed to build declaration '{}'",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<std::string>(
            log_error_message_code::failed_to_build_root,
            "failed to build global scope for '{}'",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<>(
            log_error_message_code::internal_compiler_error,
            "internal compiler error",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<std::string>(
            log_error_message_code::missing_declaration_scope,
            "'{}' must have a scope",
            ELEMENT_ERROR_INVALID_EXPRESSION,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<>(
            log_error_message_code::expression_chain_cannot_be_empty,
            "an expression chain cannot be empty",
            ELEMENT_ERROR_INVALID_EXPRESSION,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<>(
            log_error_message_code::invalid_function_declaration,
            "invalid function declaration",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<>(
            log_error_message_code::invalid_call_expression_placement,
            "call expression cannot appear as the first item in an expression chain",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<>(
            log_error_message_code::invalid_literal_expression_placement,
            "literal expression cannot appear as anything other than the first item in an expression chain",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        //todo: element_result
        register_log_error<>(
            log_error_message_code::empty_expression,
            "no expression found",
            ELEMENT_ERROR_UNKNOWN,
            ELEMENT_STAGE_PARSER);

        return true;
    }
}
