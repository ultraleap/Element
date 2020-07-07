#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>

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
        [[nodiscard]] virtual std::shared_ptr<object> index(const compilation_context& context, const identifier&) const;
        /*
         * struct declaration, function declaration, function instance
         */
        [[nodiscard]] virtual std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const;
        /*
         * expression, anything that remains after an expression is compiled, anything that a user tries to compile using the C API
         */
        [[nodiscard]] virtual std::shared_ptr<object> compile(const compilation_context& context) const;

    protected:
        object() = default;
    };

    class error : public object, public std::enable_shared_from_this<error>
    {
    public:
        explicit error(std::string message, element_result code, const declaration* location)
            : message{ std::move(message) }
            , code(code)
            , location(location)
        {

        }

        explicit error(std::string message, element_result code, const declaration* location, std::shared_ptr<error> err)
            : message(std::move(message))
            , code(code)
            , location(location)
            , err(std::move(err))
        {
            
        }

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&) const override { return const_cast<error*>(this)->shared_from_this(); };
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const override { return const_cast<error*>(this)->shared_from_this(); };
        [[nodiscard]] std::shared_ptr<object> compile(const compilation_context& context) const override { return const_cast<error*>(this)->shared_from_this(); };

        [[nodiscard]] element_result get_result() const;
        [[nodiscard]] const std::string& get_message() const;
        [[nodiscard]] const declaration* get_declaration() const;
        [[nodiscard]] const std::shared_ptr<error>& get_wrapped_error() const;

    private:
        std::string message;
        element_result code = ELEMENT_ERROR_UNKNOWN;
        const declaration* location;
        std::shared_ptr<error> err;
    };
 }
