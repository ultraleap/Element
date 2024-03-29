#pragma once

//SELF
#include "expression.hpp"
#include "common_internal.hpp"

//LIBS
#include <fmt/format.h>

namespace element
{
class literal_expression final : public expression
{
public:
    literal_expression(element_value value, const expression_chain* parent);

    [[nodiscard]] std::string to_code(const int depth = 0) const override { return fmt::format("{:g}", value); }
    [[nodiscard]] object_const_shared_ptr resolve(const compilation_context& context, const object* obj) override;

    element_value value;

private:
};
} // namespace element