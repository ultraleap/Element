#include "list_wrapper.hpp"

//STD
#include <cassert>
#include <algorithm>

//SELF
#include "object_model/error.hpp"
#include "object_model/constraints/constraint.hpp"
#include "instruction_tree/instructions.hpp"
#include "object_model/compilation_context.hpp"

using namespace element;

object_const_shared_ptr list_wrapper::create_or_optimise(const element_interpreter_ctx& interpreter,
    const object_const_shared_ptr& selector_object,
    const std::vector<object_const_shared_ptr>& option_objects,
    const source_information& source_info)
{
    if (selector_object->is_error())
        return selector_object;

    const auto* selector_constant = dynamic_cast<const element::instruction_constant*>(selector_object.get());
    if (selector_constant) {
        assert(!option_objects.empty());
        int index = static_cast<int>(selector_constant->value());
        index = std::clamp(index, 0, static_cast<int>(option_objects.size()) - 1);
        return option_objects[index];
    }

    auto selector = std::dynamic_pointer_cast<const instruction>(selector_object);
    if (!selector || (selector->actual_type != type::num.get() && selector->actual_type != type::boolean.get())) {
        return std::make_shared<const error>(
            "Tried to create a selector but it must be of type 'Num' or 'Bool'\nnote: selector is \"" + selector_object->to_string() + "\"",
            ELEMENT_ERROR_UNKNOWN,
            source_info); //todo: pass logger from context
    }

    //the selector is not constant, so all types must be homogenous
    //I'm not sure why this restriction exists in element
    const auto* actual_type = option_objects[0]->get_constraint();
    for (const auto& obj : option_objects) {
        if (obj->get_constraint() != actual_type) {
            auto error_msg = "All elements within a list must be of the same type, if that list is ever indexed with a runtime value."
                             "\nnote: The element at index '"
                             + std::to_string(option_objects.size() - 1)
                             + "' is of a different type to the first element. The first element of the list is '"
                             + option_objects[0]->to_string()
                             + "' but the element at index '"
                             + std::to_string(option_objects.size() - 1)
                             + "' is '"
                             + option_objects.back()->to_string()
                             + "'.";
            return std::make_shared<const error>(
                std::move(error_msg),
                ELEMENT_ERROR_CONSTRAINT_NOT_SATISFIED,
                source_info); //todo: pass logger from context
        }
    }

    //note: do not serialize to an instruction
    bool list_elements_are_instructions = true;
    for (const auto& element : option_objects) {
        if (!dynamic_cast<const instruction*>(element.get()))
            list_elements_are_instructions = false;
    }

    if (list_elements_are_instructions) {
        //if the list only contains one instruction then we can optimise it to be that instruction
        if (option_objects.size() == 1)
            return option_objects[0];

        std::vector<instruction_const_shared_ptr> options;
        options.reserve(option_objects.size());
        for (const auto& option : option_objects) {
            options.push_back(std::dynamic_pointer_cast<const instruction>(option));
        }

        return interpreter.cache_instruction_select.get(std::move(selector), std::move(options));
    }

    return std::make_shared<const list_wrapper>(std::move(selector), option_objects);
}

list_wrapper::list_wrapper(std::shared_ptr<const instruction> selector, std::vector<object_const_shared_ptr> options)
    : selector(std::move(selector))
    , options(std::move(options))
{
    assert(this->selector);
    const auto* first_constraint = this->options[0]->get_constraint();
    for (const auto& option : this->options) {
        assert(option);
        assert(option->get_constraint() == first_constraint);
    }
}

std::string list_wrapper::get_name() const
{
    std::string name;
    for (unsigned int i = 0; i < options.size(); ++i) {
        name += options[i]->get_name();
        if (i != options.size() - 1)
            name += " or ";
    }

    return name;
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

    return create_or_optimise(*context.interpreter, selector, new_options, source_info);
}

object_const_shared_ptr list_wrapper::index(const compilation_context& context,
    const identifier& name,
    const source_information& source_info) const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->index(context, name, source_info));

    return create_or_optimise(*context.interpreter, selector, new_options, source_info);
}

object_const_shared_ptr list_wrapper::compile(const compilation_context& context,
    const source_information& source_info) const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->compile(context, source_info));

    return create_or_optimise(*context.interpreter, selector, new_options, source_info);
}

std::shared_ptr<const instruction> list_wrapper::to_instruction(const element_interpreter_ctx& interpreter) const
{
    std::vector<object_const_shared_ptr> new_options;
    new_options.reserve(options.size());
    for (const auto& option : options)
        new_options.push_back(option->to_instruction(interpreter));

    auto result = create_or_optimise(interpreter, selector, new_options, {});
    return std::dynamic_pointer_cast<const instruction>(std::move(result));
}