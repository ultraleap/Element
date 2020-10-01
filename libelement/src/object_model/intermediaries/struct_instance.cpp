#include "struct_instance.hpp"

//STD
#include <cassert>

//SELF
#include "object_model/declarations/struct_declaration.hpp"
#include "object_model/constraints/constraint.hpp"
#include "etree/expressions.hpp"

using namespace element;

struct_instance::struct_instance(const struct_declaration* declarer)
    : declarer(declarer)
{
}

std::string struct_instance::to_string() const
{
    std::string input_string;
    for (std::size_t i = 0; i < declarer->inputs.size(); ++i)
    {
        input_string += fmt::format("{}{} = {}",
                                    declarer->inputs[i].get_name(),
                                    declarer->inputs[i].has_annotation() ? ":" + declarer->inputs[i].get_annotation()->to_string() : "",
                                    fields.at(declarer->inputs[i].get_name())->to_string());

        if (i < static_cast<int>(declarer->inputs.size()) - 1)
            input_string += ", ";
    }

    return fmt::format("{}({})", declarer->name.value, std::move(input_string));
}

bool struct_instance::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    return declarer->get_constraint()->matches_constraint(context, constraint);
}

const constraint* struct_instance::get_constraint() const
{
    return declarer->get_constraint();
}

struct_instance::struct_instance(const struct_declaration* declarer, const std::vector<object_const_shared_ptr>& expressions)
    : declarer(declarer)
{
    //TODO: JM - variadics
    assert(declarer->inputs.size() == expressions.size());
    for (size_t i = 0; i < declarer->inputs.size(); ++i)
    {
        fields.emplace(declarer->inputs[i].get_name(), expressions[i]);
    }
}

object_const_shared_ptr struct_instance::index(const compilation_context& context, const identifier& name,
                                               const source_information& source_info) const
{
    const auto found_field = fields.find(name.value);

    //found it as a field
    if (found_field != fields.end())
        return found_field->second;

    return index_type(declarer, shared_from_this(), context, name, source_info);
}

object_const_shared_ptr struct_instance::compile(const compilation_context& context,
                                                 const source_information& source_info) const
{
    return shared_from_this();
}

std::shared_ptr<const element_expression> struct_instance::to_expression() const
{
    //todo: I don't think the expression needs names for the dependents, it's just index based
    std::vector<std::pair<std::string, expression_const_shared_ptr>> dependents;
    for (const auto& input : declarer->inputs)
    {
        assert(fields.count(input.get_name()));
        const auto field = fields.at(input.get_name());
        assert(field);

        auto expr = field->to_expression();
        if (!expr)
            return nullptr;

        dependents.push_back({ input.get_name(), std::move(expr) });
    }

    return std::make_shared<element_expression_structure>(std::move(dependents));
}