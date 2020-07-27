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
        empty_expression

    };

    template <typename... Args>
    struct log_error_map
    {
        using func = std::function<log_message(const source_information& source_info, Args...)>;
        using map = std::map<log_error_message_code, func>;

        static inline map func_map;

        static void register_func(log_error_message_code code, func&& b)
        {
            auto [it, success] = func_map.insert({ code, std::move(b) });
            if (!success)
                throw;
        }

        static log_message build_error(const source_information& source_info, log_error_message_code code, Args... args)
        {
            auto it = func_map.find(code);
            if (it == func_map.end())
            {
                //todo: not valid so do something better
                throw;
            }

            return it->second(source_info, args...);
        }
    };

    //specialisation for zero parameters
    template <>
    struct log_error_map<> {
        using func = std::function<log_message(const source_information&)>;
        using map = std::map<log_error_message_code, func>;

        static inline map func_map;

        static void register_func(log_error_message_code code, func&& b)
        {
            auto [it, success] = func_map.insert({ code, std::move(b) });
            if (!success)
                throw;
        }

        static log_message build_error(const source_information& source_info, log_error_message_code code)
        {
            const auto it = func_map.find(code);
            if (it == func_map.end())
            {
                //todo: not valid so do something better
                throw;
            }

            return it->second(source_info);
        }
    };

    //template helper methods
    template <typename... Args>
    void register_log_error(log_error_message_code code, std::string format, element_result error_result, element_stage stage)
    {
        auto builder = [f = std::move(format), error_result, stage](const source_information& source_info, Args... args)
        {
            element_log_message msg;
            msg.line_in_source = source_info.line_in_source ? source_info.line_in_source->c_str() : nullptr;
            msg.character = source_info.character_start;
            msg.filename = source_info.filename;
            msg.message_code = error_result;
            msg.line = source_info.line;
            msg.stage = stage;
            msg.related_log_message = nullptr; //todo: delete?
            msg.length = source_info.character_end - source_info.character_start; //todo: UTF8 concerns?
            msg.message = nullptr;

            auto our_string = fmt::format(f, args...);
            return log_message(std::move(msg), std::move(our_string));
        };

        log_error_map<Args...>::register_func(code, std::move(builder));
    }

    //build_error is intended to fall back on template argument deduction to avoid the need for silly template parameters everywhere

    template <typename... Args>
    log_message build_log_error(const source_information& source_info, log_error_message_code code, Args... args)
    {
        return log_error_map<Args...>::build_error(source_info, code, args...);
    }

    inline source_information build_source_info(const source_context* context, const element_token* token, int extra_length)
    {
        source_information source_info;

        if (context && token)
        {
            const auto& file_info = context->file_info.at(token->file_name);
            const std::string* filename = file_info.file_name.get();
            const int actual_line = token->line - 1;
            const std::string* line_in_source = actual_line >= 0 ? file_info.source_lines[actual_line].get() : nullptr;
            source_info = source_information(
                token->line,
                token->character,
                token->character + token->tok_len + extra_length,
                line_in_source,
                filename->data());
        }

        return source_info;
    }

    inline source_information build_source_info(const source_context* context, const element_token* token)
    {
        return build_source_info(context, token, 0);
    }

    template <typename... Args>
    log_message build_log_error(const source_context* context, const element_token* token, log_error_message_code code, Args... args)
    {
        return log_error_map<Args...>::build_error(build_source_info(context, token), code, args...);
    }

    template <typename... Args>
    log_message build_log_error(const source_context* context, const element_ast* ast, log_error_message_code code, Args... args)
    {
        if (ast && ast->nearest_token)
        {
            return build_log_error(context, ast->nearest_token, code, std::forward<Args>(args)...);
        }

        return log_error_map<Args...>::build_error(source_information{}, code, args...);
    }

    template <typename... Args>
    element_result log_error(const element_log_ctx* logger, Args&&... args)
    {
        const auto error = build_log_error(args...);
        logger->log(error);
        return error.result;
    }

    template <typename... Args>
    element_result log_error(const element_interpreter_ctx* context, Args&&... args)
    {
        return log_error(context->logger.get(), std::forward<Args>(args)...);
    }

    namespace detail
    {
        //used for initializing the errors in maps statically
        bool register_log_errors();
    }
}