#include "literal_expression.hpp"

//SELF
#include "instruction_tree/instructions.hpp"
#include "interpreter_internal.hpp"
#include "object_model/compilation_context.hpp"

using namespace element;

literal_expression::literal_expression(element_value value, const expression_chain* parent)
    : expression(parent)
    , value(value)
{
    assert(parent);
}

[[nodiscard]] object_const_shared_ptr literal_expression::resolve(const compilation_context& context, const object* obj)
{
    return context.interpreter->cache_instruction_constant.get(value, element::type::num.get());
}