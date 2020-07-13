#pragma once

//STD
#include <string>
#include <map>
#include <functional>
#include <cassert>

//SELF
#include "element/token.h"
#include "element/common.h"
#include "etree/fwd.hpp"

struct element_expression;
struct element_tokeniser_ctx;
struct element_interpreter_ctx;
struct element_parser_ctx;
struct element_ast;
namespace element
{
    class log_message;
}

#define ENSURE_NOT_NULL(t) if (t == nullptr) { return ELEMENT_ERROR_INVALID_PTR; }

using LogCallback = void (*)(const element_log_message* const);

struct element_log_ctx
{
    LogCallback callback;
	
    void log(const std::string& message, const element_stage stage) const;
    void log(const element_tokeniser_ctx& context, element_result code, const std::string& message, int length = 0, element_log_message* related_message = nullptr) const;
    void log(const element_interpreter_ctx& context, element_result code, const std::string& message, const std::string& filename) const;
    void log(const element_parser_ctx& context, element_result code, const std::string& message, const element_ast* nearest_ast = nullptr) const;
    void log(const element_compiler_ctx& context, element_result code, const std::string& message, const element_ast* nearest_ast = nullptr) const;
    void log(const element_log_message& log) const;
    void log(const element::log_message& log) const;
};

std::string tokens_to_string(const element_tokeniser_ctx* context, const element_token* nearest_token = nullptr);
std::string ast_to_string(const element_ast* ast, int depth = 0, const element_ast* ast_to_mark = nullptr);
std::string expression_to_string(const element_expression& expression, int depth = 0);
std::string ast_to_code(const element_ast* node, const element_ast* parent = nullptr, bool skip = false);

namespace element
{
    class log_message
    {
    public:
        log_message(element_log_message log_msg, std::string msg)
            : log_msg(std::move(log_msg))
            , msg(std::move(msg))
            , result(log_msg.message_code)
        {
            log_msg.message = msg.c_str();
        }

        [[nodiscard]] const element_log_message& get_log_message() const
        {
            return log_msg;
        }

        const element_result result;

    private:
        element_log_message log_msg;
        const std::string msg;
    };

    struct file_information
    {
        //note: unique_ptr so it's on the heap and the memory address doesn't change
        std::vector<std::unique_ptr<std::string>> source_lines;
        std::unique_ptr<std::string> file_name;
    };

    //after talking to james we can/should change this around, something to discuss another time
    struct source_context
    {
        std::map<const char*, file_information> file_info;
    };
}