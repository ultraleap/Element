#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <cassert>

//SELF
#include "fwd.hpp"
#include "ast/fwd.hpp"
#include "source_information.hpp"

struct element_expression;
struct element_interpreter_ctx;

namespace element
{
    struct identifier;
    class intrinsic;
    class error;
    class compilation_context;

    class call_stack
    {
    public:
        struct frame
        {
            const declaration* function;
            std::vector<std::shared_ptr<object>> compiled_arguments;
        };

        frame& push(const declaration* function, std::vector<std::shared_ptr<object>> compiled_arguments);
        void pop();

        [[nodiscard]] bool is_recursive(const declaration* declaration) const;
        [[nodiscard]] std::shared_ptr<error> build_recursive_error(
            const declaration* decl,
            const compilation_context& context,
            const source_information& source_info);

        //todo: private
        std::vector<frame> frames;
    };

    class capture_stack
    {
    public:
        struct frame
        {
            const declaration* function;
            std::vector<std::shared_ptr<object>> compiled_arguments;
        };

        capture_stack(const declaration* function, const call_stack& calls);

        [[nodiscard]] std::shared_ptr<object> find(const scope* s, const identifier& name);

        //todo: private
        std::vector<frame> frames;
    };

    class compilation_context
    {
    private:
        const scope* global_scope;

    public:
        explicit compilation_context(const scope* scope, element_interpreter_ctx* interpreter);

        [[nodiscard]] const scope* get_global_scope() const { return global_scope; }

        mutable call_stack calls;
        mutable source_information source_info;

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
        [[nodiscard]] virtual std::shared_ptr<object> index(const compilation_context& context, const identifier& name, const source_information& source_info) const;
        /*
         * struct declaration, function declaration, function instance
         */
        [[nodiscard]] virtual std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args, const source_information&
                                                           source_info) const;
        /*
         * expression, anything that remains after an expression is compiled, anything that a user tries to compile using the C API
         */
        [[nodiscard]] virtual std::shared_ptr<object> compile(const compilation_context& context, const source_information& source_info) const;

        source_information source_info;

    protected:
        object() = default;
    };

    class error final : public object, public std::enable_shared_from_this<error>
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

        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&, const source_information& source_info) const override { return const_cast<error*>(this)->shared_from_this(); };
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args, const source_information& source_info) const override { return const_cast<error*>(this)->shared_from_this(); };
        [[nodiscard]] std::shared_ptr<object> compile(const compilation_context& context, const source_information& source_info) const override { return const_cast<error*>(this)->shared_from_this(); };

        [[nodiscard]] element_result get_result() const;
        [[nodiscard]] const std::string& get_message() const;
        [[nodiscard]] element_log_message get_log_message() const;

    private:
        std::string message;
        element_result code = ELEMENT_ERROR_UNKNOWN;
    };

    bool valid_call(const declaration* declarer, const std::vector<std::shared_ptr<object>>& compiled_args);
    std::shared_ptr<error> build_error_for_invalid_call(const declaration* declarer, const std::vector<std::shared_ptr<object>>& compiled_args);
    std::shared_ptr<object> index_type(const declaration* type, std::shared_ptr<object> instance, const compilation_context& context, const identifier& name, const source_information& source_info);
 }
