#include "expressions.hpp"

//LIBS
#include <fmt/format.h>

//SELF
//#include "obj_model/errors.hpp"
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
        //TODO: Handle as error
        assert(!"failed to index on type, user error?");
        return nullptr;
        //return element::build_error(element::error_message_code::not_indexable);
    }

    auto* func = dynamic_cast<element::function_declaration*>(declarer.get());

    //todo: not real typechecking. also, kinda duplicated from struct instance.
    const bool has_inputs = func && func->has_inputs();
    const bool has_type = has_inputs && func->inputs[0].annotation.get();
    const bool types_match = has_type && func->inputs[0].annotation->name.value == actual_type->get_identifier().value;

    if (types_match)
    {
        std::vector<std::shared_ptr<object>> args = { const_cast<element_expression*>(this)->shared_from_this() };
        return std::make_shared<element::function_instance>(func, context.stack, args);
    }

    if (func)
    {
        if (!has_inputs)
        {
            return std::make_shared<element::error>(
                fmt::format("error: '{}' was found when indexing '{}' but it is not an instance function as it has no parameters.\n",
                    func->typeof_info(), typeof_info()),
                ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION,
                source_info);
        }

        if (!has_type)
        {
            return std::make_shared<element::error>(
                fmt::format("error: '{}' was found when indexing '{}' but it is not an instance function as it does not have an explicit type defined for its first parameter '{}'.\n",
                    func->typeof_info(), typeof_info(), func->inputs[0].name.value),
                ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION,
                source_info);
        }

        if (!types_match)
        {
            return std::make_shared<element::error>(
                fmt::format("error: '{}' was found when indexing '{}' but it is not an instance function as the first parameter '{}' is of type '{}', when it needs to be '{}' to be considered an instance function.\n",
                    func->typeof_info(), typeof_info(), func->inputs[0].name.value, func->inputs[0].annotation->name.value, actual_type->get_identifier().value),
                ELEMENT_ERROR_CANNOT_BE_USED_AS_INSTANCE_FUNCTION,
                source_info);
        }
    }

    return nullptr;
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