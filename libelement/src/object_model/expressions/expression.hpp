#pragma once

//STD
#include <cassert>
#include <string>

//SELF
#include "object_model/fwd.hpp"
#include "source_information.hpp"

namespace element
{
class expression
{
public:
    explicit expression(const expression_chain* parent)
        : parent(parent)
    {
    }

    virtual ~expression() = default;

    //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
    expression(const expression&) = delete;
    expression(expression&&) = delete;
    expression& operator=(const expression&) = delete;
    expression& operator=(expression&&) = delete;

    [[nodiscard]] virtual std::string to_code(const int depth = 0) const = 0;
    [[nodiscard]] virtual object_const_shared_ptr resolve(const compilation_context& context, const object* obj) = 0;

    source_information source_info;

protected:
    const expression_chain* parent;
};
} // namespace element