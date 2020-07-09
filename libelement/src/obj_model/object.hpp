#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <cassert>

//SELF
#include "fwd.hpp"
#include "element/interpreter.h"

struct element_expression;
struct element_interpreter_ctx;

namespace element
{
    struct identifier;
    class intrinsic;

    class call_stack
    {
    public:
        struct frame
        {
            const declaration* function;
            std::vector<std::shared_ptr<object>> compiled_arguments;
        };

        std::vector<frame> frames;
    };

    class compilation_context
    {
    private:
        const scope* global_scope;

    public:
        explicit compilation_context(const scope* scope, element_interpreter_ctx* interpreter);

        [[nodiscard]] const scope* get_global_scope() const { return global_scope; }
        [[nodiscard]] bool is_recursive(const declaration* declaration) const
        {
            for (auto it = stack.frames.rbegin(); it != stack.frames.rend(); ++it)
            {
                if (it->function == declaration)
                    return true;
            }

            return false;
        }
        mutable call_stack stack;
        element_interpreter_ctx* interpreter;
    };

    struct identifier
    {
        identifier() = default;

        identifier(std::string value)
            : value{std::move(value)}
        {
        }

        static identifier return_identifier;

        identifier(identifier const& other) = default;
        identifier& operator=(identifier const& other) = default;

        identifier(identifier&& other) = default;
        identifier& operator=(identifier&& other) = default;

        ~identifier() = default;

        std::string value;

        bool operator <(const identifier& rhs) const
        {
            return value < rhs.value;
        }
    };

    struct source_information
    {
    public:
        source_information()
        {
            
        }

        source_information(int line, int character_start, int character_end, const std::string* line_in_source, const char* filename)
            : line(line)
        , character_start(character_start)
        , character_end(character_end)
        , line_in_source(line_in_source)
        , filename(filename)
        {}

        const std::string& get_text()
        {
            if (text.empty())
            {
                //todo: UTF8 concerns?
                assert(character_end - character_start >= 0);
                text = line_in_source->substr(character_start - 1, character_end - character_start);
            }

            return text;
        }

        int line = 0;
        int character_start = 0;
        int character_end = 0;

        const std::string* line_in_source = nullptr;
        const char* filename = nullptr;

    private:
        std::string text;
    };

    class object
    {
    public:
        virtual ~object() = default;

        [[nodiscard]] virtual std::string typeof_info() const = 0;
        [[nodiscard]] virtual std::string to_code(int depth) const = 0;

        //TODO: Add constraints
        //bool matches_constraint(constraint& constraint);

        /*
         * Namespace, element_expression, struct declaration, struct instance, function declaration if nullary
         */
        [[nodiscard]] virtual std::shared_ptr<object> index(const compilation_context& context, const identifier& name) const;
        /*
         * struct declaration, function declaration, function instance
         */
        [[nodiscard]] virtual std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const;
        /*
         * expression, anything that remains after an expression is compiled, anything that a user tries to compile using the C API
         */
        [[nodiscard]] virtual std::shared_ptr<object> compile(const compilation_context& context) const;

        source_information source_info;

    protected:
        object() = default;
    };

    class error : public object, public std::enable_shared_from_this<error>
    {
    public:
        explicit error(std::string message, element_result code, source_information src_info)
            : message{ std::move(message) }
            , code(code)
        {
            source_info = std::move(src_info);
        }

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&) const override { return const_cast<error*>(this)->shared_from_this(); };
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const override { return const_cast<error*>(this)->shared_from_this(); };
        [[nodiscard]] std::shared_ptr<object> compile(const compilation_context& context) const override { return const_cast<error*>(this)->shared_from_this(); };

        [[nodiscard]] element_result get_result() const;
        [[nodiscard]] const std::string& get_message() const;
        [[nodiscard]] const element_log_message get_log_message() const;

    private:
        std::string message;
        element_result code = ELEMENT_ERROR_UNKNOWN;
    };
 }
