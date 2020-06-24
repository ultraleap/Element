#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <cassert>

//SELF
#include "fwd.hpp"

struct element_expression;

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
        explicit compilation_context(const scope* scope);

        [[nodiscard]] const scope* get_global_scope() const { return global_scope; }
        [[nodiscard]] bool is_recursive(const declaration* declaration) const
        {
            for (auto it = stack.frames.rbegin(); it != stack.frames.rend(); it++)
            {
                if (it->function == declaration)
                    return true;
            }

            return false;
        }
        mutable call_stack stack;
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

        [[nodiscard]] virtual std::string to_string() const { assert(!"make abs"); throw;  return "make abs"; }
        [[nodiscard]] virtual std::string to_code(int depth) const { assert(!"make abs"); throw;  return "make abs"; }

        //TODO: Add constraints
        //bool matches_constraint(constraint& constraint);

        /*
         * Namespace, element_expression, struct declaration, struct instance, function declaration if nullary
         */
        [[nodiscard]] virtual std::shared_ptr<object> index(const compilation_context& context, const identifier&) const { assert(!"make abs"); throw;  return nullptr; };
        /*
         * struct declaration, function declaration, function instance
         */
        [[nodiscard]] virtual std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const { assert(!"make abs"); throw;  return nullptr; };
        /*
         * expression, anything that remains after an expression is compiled, anything that a user tries to compile using the C API
         */
        [[nodiscard]] virtual std::shared_ptr<object> compile(const compilation_context& context) const { assert(!"make abs"); throw;  return nullptr; };

    protected:
        object() = default;
    };

    //struct error : object
    // {
    //    static const object_model_id type_id;
    //
    //    std::string message;

    //    explicit error(std::string message)
    //        : object(nullptr, type_id)
    //        , message{std::move(message)}
    //    {
    //    }
    //};
 }
