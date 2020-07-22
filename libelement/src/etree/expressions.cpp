#include "expressions.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "obj_model/errors.hpp"
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

std::shared_ptr<element::object> element_expression::compile(const element::compilation_context& context, const element::source_information& source_info) const
{
    return const_cast<element_expression*>(this)->shared_from_this();
}

std::shared_ptr<element::object> element_expression::index(
    const element::compilation_context& context,
    const element::identifier& name,
    const element::source_information& source_info) const
{
    if (!actual_type)
    {
        assert(false);
        return nullptr;
    }

    //find the declaration of the type that we are
    const auto actual_type_decl = context.get_global_scope()->find(actual_type->get_identifier(), false);
    if (!actual_type_decl)
    {
        //TODO: Handle as error
        assert(!"failed to find declaration of our actual type, did the user declare the intrinsic?");
        return nullptr;
    }

    return index_type(actual_type_decl.get(), const_cast<element_expression*>(this)->shared_from_this(), context, name, source_info);
}

element_expression_constant::element_expression_constant(element_value val)
    : element_expression(type_id, element::type::num.get())
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