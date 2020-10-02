#include "expressions.hpp"

#include <utility>

//SELF
#include "object_model/compilation_context.hpp"
#include "object_model/error.hpp"

DEFINE_TYPE_ID(element_expression_constant, 1U << 0);
DEFINE_TYPE_ID(element_expression_input, 1U << 1);
DEFINE_TYPE_ID(element_expression_serialised_structure, 1U << 2);
DEFINE_TYPE_ID(element_expression_nullary, 1U << 3);
DEFINE_TYPE_ID(element_expression_unary, 1U << 4);
DEFINE_TYPE_ID(element_expression_binary, 1U << 5);
DEFINE_TYPE_ID(element_expression_if, 1U << 6);
DEFINE_TYPE_ID(element_expression_select, 1U << 7);
DEFINE_TYPE_ID(element_expression_indexer, 1U << 8);
DEFINE_TYPE_ID(element_expression_for, 1U << 9);
DEFINE_TYPE_ID(element_expression_fold, 1U << 10);

std::shared_ptr<const element::object> element_expression::compile(const element::compilation_context& context, const element::source_information& source_info) const
{
    return shared_from_this();
}

std::shared_ptr<const element::object> element_expression::index(
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
    const auto* const actual_type_decl = context.get_global_scope()->find(actual_type->get_identifier(), false);
    if (!actual_type_decl)
    {
        //TODO: Handle as error
        assert(!"failed to find declaration of our actual type, did the user declare the intrinsic?");
        return nullptr;
    }

    return index_type(actual_type_decl, shared_from_this(), context, name, source_info);
}

element_expression_constant::element_expression_constant(element_value val)
    : element_expression(type_id, element::type::num.get())
    , m_value(val)
{
}

element::object_const_shared_ptr element_expression_constant::call(const element::compilation_context& context, std::vector<element::object_const_shared_ptr> compiled_args, const element::source_information& source_info) const
{
    return std::make_shared<element::error>("Tried to call something that isn't a function", ELEMENT_ERROR_INVALID_CALL_NONFUNCTION, source_info);
}

element_expression_if::element_expression_if(expression_const_shared_ptr predicate, expression_const_shared_ptr if_true, expression_const_shared_ptr if_false)
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

element_expression_for::element_expression_for(expression_const_shared_ptr initial, expression_const_shared_ptr condition, expression_const_shared_ptr body)
    : element_expression(type_id, nullptr)
{
    actual_type = initial->actual_type;

    m_dependents.emplace_back(std::move(initial));
    m_dependents.emplace_back(std::move(condition));
    m_dependents.emplace_back(std::move(body));
}

element_expression_fold::element_expression_fold(expression_const_shared_ptr list, expression_const_shared_ptr initial, expression_const_shared_ptr accumulator)
    : element_expression(type_id, nullptr)
{
    actual_type = initial->actual_type;

    m_dependents.emplace_back(std::move(list));
    m_dependents.emplace_back(std::move(initial));
    m_dependents.emplace_back(std::move(accumulator));
}

element_expression_indexer::element_expression_indexer(std::shared_ptr<const element_expression_for> for_expression, int index, element::type_const_ptr type)
    : element_expression(type_id, type)
    , for_expression{ std::move(for_expression) }
    , index{ index }
{
}

element_expression_select::element_expression_select(expression_const_shared_ptr selector, std::vector<expression_const_shared_ptr> options)
    : element_expression(type_id, nullptr)
    , selector(std::move(selector))
    , options(std::move(options))
{
    assert(this->selector);
    for (auto& option : this->options)
    {
        assert(option);
    }
}