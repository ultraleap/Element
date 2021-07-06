#pragma once

//STD
#include <string>
#include <vector>

//SELF
#include "expression.hpp"

namespace element
{
class call_expression final : public expression
{
public:
    call_expression(const expression_chain* parent);

    [[nodiscard]] std::string to_code(const int depth = 0) const override;
    [[nodiscard]] object_const_shared_ptr resolve(const compilation_context& context, const object* obj) override;

    std::vector<std::unique_ptr<expression_chain>> arguments;

private:
};
} // namespace element