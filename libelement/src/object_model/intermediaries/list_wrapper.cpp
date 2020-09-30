#include "list_wrapper.hpp"

//STD
#include <cassert>
#include <algorithm>

//SELF
#include "object_model/error.hpp"
#include "object_model/constraints/constraint.hpp"
#include "etree/expressions.hpp"

using namespace element;

object_const_shared_ptr list_wrapper::create_or_optimise(const object_const_shared_ptr& selector_object,
                                                         const std::vector<object_const_shared_ptr>& option_objects,
                                                         const source_information& source_info)
{
    auto index_error = std::dynamic_pointer_cast<const error>(selector_object);
    if (index_error)
        return index_error;

    const auto* selector_constant = dynamic_cast<const element_expression_constant*>(selector_object.get());
    if (selector_constant)
    {
        assert(!option_objects.empty());
        int index = selector_constant->value();
        index = std::clamp(index, 0, static_cast<int>(option_objects.size()) - 1);
        return option_objects[index];
    }

    auto selector = std::dynamic_pointer_cast<const element_expression>(selector_object);
    if (!selector || selector->actual_type != type::num.get())
    {
        return std::make_shared<const error>(
            "Tried to create a selector but it must be of type 'Num'\nnote: typeof selector is \"" + selector_object->typeof_info() + "\"",
            ELEMENT_ERROR_UNKNOWN, source_info);
    }

    //the selector is not constant, so all types must be homogenous
    //I'm not sure why this restriction exists in element
    const auto* actual_type = option_objects[0]->get_constraint();
    for (const auto& obj : option_objects)
    {
        if (obj->get_constraint() != actual_type)
        {
            return std::make_shared<const error>(
                "All elements within a list must be of the same type, if that list is ever indexed with a runtime value.\nnote: The first element of the list is of type '" + option_objects[0]->typeof_info() + "' yet element at index '" + std::to_string(option_objects.size() - 1) + "' is of type '" + option_objects.back()->typeof_info() + "'",
                ELEMENT_ERROR_UNKNOWN, source_info);
        }
    }

    //note: do not serialize to an expression
    bool list_elements_are_expressions = true;
    for (const auto& element : option_objects)
    {
        if (!dynamic_cast<const element_expression*>(element.get()))
            list_elements_are_expressions = false;
    }

    if (list_elements_are_expressions)
    {
        std::vector<expression_const_shared_ptr> options;
        options.reserve(option_objects.size());
        for (const auto& option : option_objects)
        {
            options.push_back(std::dynamic_pointer_cast<const element_expression>(option));
        }

        auto select = std::make_shared<const element_expression_select>(std::move(selector), std::move(options));
        select->actual_type = select->options[0]->actual_type;
        return select;
    }

    return std::make_shared<const list_wrapper>(std::move(selector), option_objects);
}

list_wrapper::list_wrapper(std::shared_ptr<const element_expression> selector, std::vector<object_const_shared_ptr> options)
    : selector(std::move(selector))
    , options(std::move(options))
{
    assert(this->selector);
    const auto* first_constraint = this->options[0]->get_constraint();
    for (const auto& option : this->options)
    {
        assert(option);
        assert(option->get_constraint() == first_constraint);
    }
}

std::string list_wrapper::typeof_info() const
{
    return "?";
}

std::string list_wrapper::to_code(const int depth) const
{
    return "?";
}

bool list_wrapper::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    return options[0]->matches_constraint(context, constraint);
}

const constraint* list_wrapper::get_constraint() const
{
    return options[0]->get_constraint();
}

object_const_shared_ptr list_wrapper::call(const compilation_context& context,
                                           std::vector<object_const_shared_ptr> compiled_args,
                                           const source_information& source_info) const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->call(context, compiled_args, source_info));

    return create_or_optimise(selector, new_options, source_info);
}

object_const_shared_ptr list_wrapper::index(const compilation_context& context,
                                            const identifier& name,
                                            const source_information& source_info) const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->index(context, name, source_info));

    return create_or_optimise(selector, new_options, source_info);
}

object_const_shared_ptr list_wrapper::compile(const compilation_context& context,
                                              const source_information& source_info) const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->compile(context, source_info));

    return create_or_optimise(selector, new_options, source_info);
}

std::shared_ptr<const element_expression> list_wrapper::to_expression() const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->to_expression());

    auto result = create_or_optimise(selector, new_options, {});
    return std::dynamic_pointer_cast<const element_expression>(std::move(result));
}