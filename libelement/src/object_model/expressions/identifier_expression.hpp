#pragma once

//SELF
#include "expression.hpp"
#include "object_model//identifier.hpp"

namespace element
{
class identifier_expression final : public expression
{
public:
    identifier_expression(identifier name, const expression_chain* parent);

    [[nodiscard]] std::string to_code(const int depth = 0) const override;
    [[nodiscard]] object_const_shared_ptr resolve(const compilation_context& context, const object* obj) override;

private:
    identifier name;
};
} // namespace element