#include "literal_expression.hpp"

//SELF
#include "etree/expressions.hpp"
#include "interpreter_internal.hpp"

using namespace element;

literal_expression::literal_expression(element_value value, const expression_chain* parent)
    : expression(parent)
    , value(value)
{
    assert(parent);
}

[[nodiscard]] object_const_shared_ptr literal_expression::resolve(const compilation_context& context, const object* obj)
{
    return std::make_shared<element_expression_constant>(value);
}