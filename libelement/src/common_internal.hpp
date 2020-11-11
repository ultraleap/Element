#pragma once

//STD
#include <string>
#include <map>
#include <functional>
#include <cassert>
#include <vector>
#include <iostream>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/token.h"
#include "element/common.h"
#include "instruction_tree/fwd.hpp"

struct element_tokeniser_ctx;
struct element_interpreter_ctx;
struct element_parser_ctx;
struct element_ast;

namespace element
{
    struct instruction;
    class log_message;
}

#define ENSURE_NOT_NULL(t)                \
    if (t == nullptr)                     \
    {                                     \
        return ELEMENT_ERROR_INVALID_PTR; \
    }

using LogCallback = void (*)(const element_log_message*, void*);

struct element_log_ctx
{
    LogCallback callback = nullptr;
    void* user_data = nullptr;

    void log(const std::string& message, const element_stage stage) const;
    void log(const element_tokeniser_ctx& context, element_result code, const std::string& message, int length = 0, element_log_message* related_message = nullptr) const;
    void log(const element_interpreter_ctx& context, element_result code, const std::string& message, const std::string& filename) const;
    void log(const element_interpreter_ctx& context, element_result code, const std::string& message) const;
    void log(const element_parser_ctx& context, element_result code, const std::string& message, const element_ast* nearest_ast = nullptr) const;
    void log(const element_compiler_ctx& context, element_result code, const std::string& message, const element_ast* nearest_ast = nullptr) const;
    void log(const element_log_message& log) const;
    void log(const element::log_message& log) const;

    template <typename String, typename... Args>
    void log_step(String&& str, Args&&... args) const
    {
        try
        {
            fmt::print(std::string(compilation_step_indent_level, '\t') + std::forward<String>(str), std::forward<Args>(args)...);
        }
        catch (std::exception& e)
        {
            fmt::print(std::string("Failed to log_step - ") + e.what());
        }
        catch (...)
        {
            std::cout << "Failed to log_step\n";
        }
    }

    void log_step_indent() const
    {
        compilation_step_indent_level += 1;
    }

    void log_step_unindent() const
    {
        if (compilation_step_indent_level > 0)
            compilation_step_indent_level -= 1;
    }

    int log_step_get_indent_level() const
    {
        return compilation_step_indent_level;
    }

    mutable int compilation_step_indent_level = 0;
};

std::string tokens_to_string(const element_tokeniser_ctx* context, const element_token* nearest_token = nullptr);
std::string ast_to_string(const element_ast* ast, int depth = 0, const element_ast* ast_to_mark = nullptr);
std::string instruction_to_string(const element::instruction& expression, std::size_t depth = 0);
std::string ast_to_code(const element_ast* node, const element_ast* parent = nullptr);

namespace element
{
    class log_message
    {
    public:
        log_message(element_log_message log_msg, std::string msg)
            : result(log_msg.message_code)
            , log_msg(std::move(log_msg))
            , msg(std::move(msg))
        {
            this->log_msg.message = this->msg.c_str();
            this->log_msg.message_length = static_cast<int>(this->msg.length());
        }

        [[nodiscard]] const element_log_message& get_log_message() const
        {
            return log_msg;
        }

        void append_text(std::string txt)
        {
            msg += txt;
            this->log_msg.message = this->msg.c_str();
            this->log_msg.message_length = static_cast<int>(this->msg.length());
        }

        const element_result result;

    private:
        element_log_message log_msg;
        std::string msg;
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
} // namespace element