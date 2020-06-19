#pragma once

//STD
#include <utility>
#include <vector>

//SELF
#include "element/common.h"
#include "types.hpp"
#include "object.hpp"
#include "port.hpp"
#include "fwd.hpp"

namespace element
{
    class expression : public object, public std::enable_shared_from_this<expression>
    {
    public:
        explicit expression(const scope* enclosing_scope);
        virtual ~expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        expression(const expression&) = delete;
        expression(expression&&) = delete;
        expression& operator=(const expression&) = delete;
        expression& operator=(expression&&) = delete;

        [[nodiscard]] virtual bool has_children() const { return !children.empty(); }
        [[nodiscard]] std::string to_code(int depth = 0) const override;
        [[nodiscard]] std::shared_ptr<compiled_expression> compile(const compilation_context& context) const;
        [[nodiscard]] virtual std::shared_ptr<object> resolve_expression(const compilation_context& context, std::shared_ptr<object>) { return nullptr; };

        const scope* enclosing_scope;

        //need to use a shared pointer here as call expressions can have a list of independent expressions that all share the same parent i.e. the call expression itself
        std::vector<std::shared_ptr<expression>> children;

    private:
    };

    class literal_expression final : public expression
    {
    public:
        explicit literal_expression(const element_value value, const scope* enclosing_scope)
            : expression{ enclosing_scope }, value{ value }
        {
        }
        virtual ~literal_expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        literal_expression(const literal_expression&) = delete;
        literal_expression(literal_expression&&) = delete;
        literal_expression& operator=(const literal_expression&) = delete;
        literal_expression& operator=(literal_expression&&) = delete;

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_code(int depth = 0) const override { return std::to_string(value); }
        [[nodiscard]] std::shared_ptr<object> resolve_expression(const compilation_context& context, std::shared_ptr<object> previous) override;

        element_value value;

    private:
    };

    class identifier_expression final : public expression
    {
    public:
        explicit identifier_expression(identifier identifier, const scope* enclosing_scope)
            : expression{ enclosing_scope }, identifier{ std::move(identifier) }
        {
        }
        virtual ~identifier_expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        identifier_expression(const identifier_expression&) = delete;
        identifier_expression(identifier_expression&&) = delete;
        identifier_expression& operator=(const identifier_expression&) = delete;
        identifier_expression& operator=(identifier_expression&&) = delete;

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_code(int depth = 0) const override { return identifier.value; }
        [[nodiscard]] std::shared_ptr<object> resolve_expression(const compilation_context& context, std::shared_ptr<object> previous) override;

        identifier identifier;

    private:
    };

    class call_expression final : public expression
    {
    public:
        explicit call_expression(const scope* enclosing_scope)
            : expression{ enclosing_scope }
        {
        }
        virtual ~call_expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        call_expression(const call_expression&) = delete;
        call_expression(call_expression&&) = delete;
        call_expression& operator=(const call_expression&) = delete;
        call_expression& operator=(call_expression&&) = delete;

        [[nodiscard]] std::string to_code(int depth = 0) const override;
        [[nodiscard]] std::shared_ptr<object> resolve_expression(const compilation_context& context, std::shared_ptr<object> previous) override;

    private:
    };

    class indexing_expression final : public expression
    {
    public:
        explicit indexing_expression(identifier name, const scope* enclosing_scope)
            : expression{ enclosing_scope }, name{ std::move(name) }
        {
        }
        virtual ~indexing_expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        indexing_expression(const indexing_expression&) = delete;
        indexing_expression(indexing_expression&&) = delete;
        indexing_expression& operator=(const indexing_expression&) = delete;
        indexing_expression& operator=(indexing_expression&&) = delete;

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_code(int depth = 0) const override { return "." + name.value; }
        [[nodiscard]] std::shared_ptr<object> resolve_expression(const compilation_context& context, std::shared_ptr<object> previous) override;

        identifier name;

    private:
    };

    class lambda_expression : public expression
    {
    public:
        lambda_expression(const scope* parent_scope);
        virtual ~lambda_expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        lambda_expression(const lambda_expression&) = delete;
        lambda_expression(lambda_expression&&) = delete;
        lambda_expression& operator=(const lambda_expression&) = delete;
        lambda_expression& operator=(lambda_expression&&) = delete;

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        std::vector<port> inputs;
        std::unique_ptr<port> output;

    private:
    };

    //expression bodied functions are used as the leaf-functions for a chain of scope bodied ones to prevent recursion
    //the last thing in a function call chain must be an expression bodied "return"
    class expression_bodied_lambda_expression final : public lambda_expression
    {
    public:
        expression_bodied_lambda_expression(const element::scope* parent_scope);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        std::shared_ptr<expression> expression;

    private:
    };
}
