#pragma once

//STD
#include <string>
#include <vector>

//SELF
#include "expression.hpp"
#include "object_model/declarations/function_declaration.hpp"

namespace element
{
    class lambda_expression final : public expression
    {
    public:
        lambda_expression(const expression_chain* parent);

        [[nodiscard]] std::string to_code(const int depth = 0) const override { return function->to_code(depth); }
        [[nodiscard]] object_const_shared_ptr resolve(const compilation_context& context, const object* obj) override;

        std::unique_ptr<function_declaration> function;

    private:
    };
} // namespace element