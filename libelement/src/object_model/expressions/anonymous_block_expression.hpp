#pragma once

//STD
#include <string>

//SELF
#include "expression.hpp"
#include "object_model/object_internal.hpp"

namespace element
{
class anonymous_block_expression final : public expression
{
public:
    anonymous_block_expression(const expression_chain* parent);

    [[nodiscard]] std::string to_code(const int depth = 0) const override;
    [[nodiscard]] object_const_shared_ptr resolve(const compilation_context& context, const object* obj) override;

    std::unique_ptr<scope> our_scope;

private:
};
} // namespace element