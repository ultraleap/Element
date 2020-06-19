#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>

//SELF
#include "fwd.hpp"

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
            std::vector<std::shared_ptr<compiled_expression>> arguments;
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

        [[nodiscard]] virtual std::string to_string() const { return ""; }
        [[nodiscard]] virtual std::string to_code(int depth) const { return ""; }

        //TODO: Add constraints
        //bool matches_constraint(constraint& constraint);

        //todo: some kind of component architecture?
        [[nodiscard]] virtual std::shared_ptr<object> index(const compilation_context& context, const identifier&) const { return nullptr; };
        [[nodiscard]] virtual std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const { return nullptr; };
        [[nodiscard]] virtual std::shared_ptr<compiled_expression> compile(const compilation_context& context) const { return nullptr; };

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
