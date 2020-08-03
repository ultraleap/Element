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
    class expression_chain final : public object
    {
    public:
        explicit expression_chain(const declaration* declarer);
        virtual ~expression_chain() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        expression_chain(const expression_chain&) = delete;
        expression_chain(expression_chain&&) = delete;
        expression_chain& operator=(const expression_chain&) = delete;
        expression_chain& operator=(expression_chain&&) = delete;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;
        [[nodiscard]] std::shared_ptr<const object> call(const compilation_context& context,
                                                         std::vector<std::shared_ptr<const object>> compiled_args,
                                                         const source_information&
                                                         source_info) const override;
        [[nodiscard]] std::shared_ptr<const object> compile(const compilation_context& context,
                                                            const source_information& source_info) const override;

        const declaration* declarer;
        std::vector<std::unique_ptr<expression>> expressions;
        mutable capture_stack captures;
    private:
    };

    class expression
    {
    public:
        explicit expression(const expression_chain* parent);
        virtual ~expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        expression(const expression&) = delete;
        expression(expression&&) = delete;
        expression& operator=(const expression&) = delete;
        expression& operator=(expression&&) = delete;

        [[nodiscard]] virtual std::string to_code(int depth = 0) const = 0;
        [[nodiscard]] virtual std::shared_ptr<const object> resolve(const compilation_context& context, const object* obj) = 0;

        source_information source_info;

    protected:
        const expression_chain* parent;
    };

    class literal_expression final : public expression
    {
    public:
        literal_expression(element_value value, const expression_chain* parent);

        [[nodiscard]] std::string to_code(int depth = 0) const override { return std::to_string(value); }
        [[nodiscard]] std::shared_ptr<const object> resolve(const compilation_context& context, const object* obj) override;

        element_value value;

    private:
    };

    class identifier_expression final : public expression
    {
    public:
        identifier_expression(identifier name, const expression_chain* parent);

        [[nodiscard]] std::string to_code(int depth = 0) const override;
        [[nodiscard]] std::shared_ptr<const object> resolve(const compilation_context& context, const object* obj) override;

    private:
        identifier name;
    };

    class call_expression final : public expression
    {
    public:
        call_expression(const expression_chain* parent);

        [[nodiscard]] std::string to_code(int depth = 0) const override;
        [[nodiscard]] std::shared_ptr<const object> resolve(const compilation_context& context, const object* obj) override;

        std::vector<std::unique_ptr<expression_chain>> arguments;
    private:
    };

    class indexing_expression final : public expression
    {
    public:
        indexing_expression(identifier name, const expression_chain* parent);

        [[nodiscard]] std::string to_code(int depth = 0) const override;
        [[nodiscard]] std::shared_ptr<const object> resolve(const compilation_context& context, const object* obj) override;

    private:
        identifier name;
    };

    //lambdas are basically function declarations and expressions, can we combine/simplify in some way?
    class lambda_expression final : public expression
    {
    public:
        lambda_expression(const expression_chain* parent);

        [[nodiscard]] std::string to_code(int depth) const override;
        [[nodiscard]] std::shared_ptr<const object> resolve(const compilation_context& context, const object* obj) override;

        //need to think about what this requires
        std::vector<port> inputs;
        std::optional<port> output;
        std::unique_ptr<scope> our_scope;
        std::shared_ptr<const object> body;

    private:
        identifier name;
    };
}
