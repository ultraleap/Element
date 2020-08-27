#include "intrinsic_compiler_list_indexer.hpp"

//SELF
#include "object_model/declarations/declaration.hpp"
#include "object_model/error.hpp"

using namespace element;

intrinsic_compiler_list_indexer::intrinsic_compiler_list_indexer()
    : intrinsic_function(type_id, nullptr)
{
}

object_const_shared_ptr intrinsic_compiler_list_indexer::compile(const compilation_context& context,
                                                                 const source_information& source_info) const
{
    const auto& compiled_arguments = context.calls.frames.back().compiled_arguments;
    const auto& compiled_list_arguments = context.captures.frames[context.captures.frames.size() - 2].compiled_arguments;

    const auto* constant_expression = dynamic_cast<const element_expression_constant*>(compiled_arguments[0].get());
    if (constant_expression)
        return compiled_list_arguments[constant_expression->value()];

    bool list_elements_are_expressions = true;
    for (const auto& element : compiled_list_arguments)
    {
        if (!dynamic_cast<const element_expression*>(element.get()))
            list_elements_are_expressions = false;
    }

    if (list_elements_are_expressions)
    {
        std::vector<expression_const_shared_ptr> options;
        options.reserve(compiled_list_arguments.size());
        for (const auto& element : compiled_list_arguments)
            options.push_back(std::dynamic_pointer_cast<const element_expression>(element));

        auto select = std::make_shared<const element_expression_select>(std::dynamic_pointer_cast<const element_expression>(compiled_arguments[0]), options);
        select->actual_type = options[0]->actual_type;
        return select;
    }

    assert(false);
    throw;
}