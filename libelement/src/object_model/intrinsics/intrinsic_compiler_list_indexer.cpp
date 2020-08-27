#include "intrinsic_compiler_list_indexer.hpp"

//STD
#include <algorithm>

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
    const auto& our_arguments = context.calls.frames.back().compiled_arguments;
    const auto& list_arguments = context.captures.frames[context.captures.frames.size() - 2].compiled_arguments;

    const auto* constant_expression = dynamic_cast<const element_expression_constant*>(our_arguments[0].get());
    if (constant_expression)
    {
        int index = constant_expression->value();
        index = std::clamp(index, 0, static_cast<int>(list_arguments.size()) - 1);
        return list_arguments[index];
    }

    //note: do not serialize to an expression
    bool list_elements_are_expressions = true;
    for (const auto& element : list_arguments)
    {
        if (!dynamic_cast<const element_expression*>(element.get()))
            list_elements_are_expressions = false;
    }

    if (list_elements_are_expressions)
    {
        std::vector<expression_const_shared_ptr> options;
        options.reserve(list_arguments.size());
        for (const auto& element : list_arguments)
            options.push_back(std::dynamic_pointer_cast<const element_expression>(element));

        auto select = std::make_shared<const element_expression_select>(std::dynamic_pointer_cast<const element_expression>(our_arguments[0]), options);
        select->actual_type = options[0]->actual_type;
        return select;
    }

    //todo: they are some kind of intermediary, e.g. struct or function instance. Need the ListElement intermediary wrapper to handle them
    assert(false);
    throw;
}