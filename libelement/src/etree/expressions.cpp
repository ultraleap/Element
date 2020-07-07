#include "expressions.hpp"

//SELF
#include "obj_model/types.hpp"
#include "obj_model/intermediaries.hpp"

DEFINE_TYPE_ID(element_expression_constant,        1U << 0);
DEFINE_TYPE_ID(element_expression_input,           1U << 1);
DEFINE_TYPE_ID(element_expression_structure,       1U << 2);
DEFINE_TYPE_ID(element_expression_nullary,         1U << 3);
DEFINE_TYPE_ID(element_expression_unary,           1U << 4);
DEFINE_TYPE_ID(element_expression_binary,          1U << 5);
DEFINE_TYPE_ID(element_expression_if,              1U << 6);
//DEFINE_TYPE_ID(element_expression_group,           1U << 7);
//DEFINE_TYPE_ID(element_expression_unbound_arg,     1U << 8);

std::shared_ptr<element::object> element_expression::compile(const element::compilation_context& context) const
{
    //TODO: THIS IS AFWUL! FIX!
    return const_cast<element_expression*>(this)->shared_from_this();
}

std::shared_ptr<element::object> element_expression::index(const element::compilation_context& context, const element::identifier& name) const
{
    assert(actual_type);

    const auto declarer = actual_type->index(context, name);

    if (!declarer)
    {
        assert(!"failed to index on type, user error?");
        return nullptr;
    }

    auto args = std::vector<std::shared_ptr<object>>();
    args.push_back(const_cast<element_expression*>(this)->shared_from_this());

    auto* function_declaration = dynamic_cast<element::function_declaration*>(declarer.get());
    //todo: not real typechecking. also, kinda duplicated from struct instance.
    if (function_declaration && function_declaration->has_inputs() && function_declaration->inputs[0].annotation && function_declaration->inputs[0].annotation->name.value == actual_type->get_identifier().value)
        return std::make_shared<element::function_instance>(function_declaration, context.stack, args);

    assert(!"tried to index on an expression but found a function that is not an instance function");
    throw;
}

element_expression_constant::element_expression_constant(element_value val)
    : element_expression(type_id, element::type::num)
    , m_value(val)
{
}

element_expression_if::element_expression_if(expression_shared_ptr predicate, expression_shared_ptr if_true, expression_shared_ptr if_false)
    : element_expression(type_id, nullptr)
{
    if (if_true->actual_type != if_false->actual_type)
    {
        assert(!"the resulting type of the two branches of an if-expression must be the same");
    }

    actual_type = if_true->actual_type;

    m_dependents.emplace_back(std::move(predicate));
    m_dependents.emplace_back(std::move(if_true));
    m_dependents.emplace_back(std::move(if_false));
}