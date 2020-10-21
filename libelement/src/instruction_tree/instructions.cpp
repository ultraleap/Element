#include "instructions.hpp"

#include <utility>

//SELF
#include "object_model/compilation_context.hpp"
#include "object_model/error.hpp"

using namespace element;

DEFINE_TYPE_ID(element::instruction_constant, 1U << 0);
DEFINE_TYPE_ID(element::instruction_input, 1U << 1);
DEFINE_TYPE_ID(element::instruction_serialised_structure, 1U << 2);
DEFINE_TYPE_ID(element::instruction_nullary, 1U << 3);
DEFINE_TYPE_ID(element::instruction_unary, 1U << 4);
DEFINE_TYPE_ID(element::instruction_binary, 1U << 5);
DEFINE_TYPE_ID(element::instruction_if, 1U << 6);
DEFINE_TYPE_ID(element::instruction_select, 1U << 7);
DEFINE_TYPE_ID(element::instruction_indexer, 1U << 8);
DEFINE_TYPE_ID(element::instruction_for, 1U << 9);
DEFINE_TYPE_ID(element::instruction_fold, 1U << 10);

std::shared_ptr<const object> instruction::compile(const compilation_context& context, const source_information& source_info) const
{
    return shared_from_this();
}

std::shared_ptr<const object> instruction::index(
    const compilation_context& context,
    const identifier& name,
    const source_information& source_info) const
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

instruction_constant::instruction_constant(element_value val)
    : instruction(type_id, type::num.get())
    , m_value(val)
{
}

object_const_shared_ptr instruction_constant::call(const compilation_context& context, std::vector<object_const_shared_ptr> compiled_args, const source_information& source_info) const
{
    return std::make_shared<error>("Tried to call something that isn't a function", ELEMENT_ERROR_INVALID_CALL_NONFUNCTION, source_info);
}

instruction_if::instruction_if(instruction_const_shared_ptr predicate, instruction_const_shared_ptr if_true, instruction_const_shared_ptr if_false)
    : instruction(type_id, nullptr)
{
    if (if_true->actual_type != if_false->actual_type)
    {
        assert(!"the resulting type of the two branches of an if-instruction must be the same");
    }

    actual_type = if_true->actual_type;

    m_dependents.emplace_back(std::move(predicate));
    m_dependents.emplace_back(std::move(if_true));
    m_dependents.emplace_back(std::move(if_false));
}

instruction_for::instruction_for(instruction_const_shared_ptr initial, instruction_const_shared_ptr condition, instruction_const_shared_ptr body)
    : instruction(type_id, nullptr)
{
    actual_type = initial->actual_type;

    m_dependents.emplace_back(std::move(initial));
    m_dependents.emplace_back(std::move(condition));
    m_dependents.emplace_back(std::move(body));
}

instruction_fold::instruction_fold(instruction_const_shared_ptr list, instruction_const_shared_ptr initial, instruction_const_shared_ptr accumulator)
    : instruction(type_id, nullptr)
{
    actual_type = initial->actual_type;

    m_dependents.emplace_back(std::move(list));
    m_dependents.emplace_back(std::move(initial));
    m_dependents.emplace_back(std::move(accumulator));
}

instruction_indexer::instruction_indexer(std::shared_ptr<const element::instruction_for> for_instruction, int index, type_const_ptr type)
    : instruction(type_id, type)
    , for_instruction{ std::move(for_instruction) }
    , index{ index }
{
}

instruction_select::instruction_select(instruction_const_shared_ptr selector, std::vector<instruction_const_shared_ptr> options)
    : instruction(type_id, nullptr)
    , selector(std::move(selector))
    , options(std::move(options))
{
    assert(this->selector);
    for (auto& option : this->options)
    {
        assert(option);
    }
}