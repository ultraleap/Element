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
    int index = static_cast<const element_expression_constant*>(compiled_arguments[0].get())->value();

    return compiled_list_arguments[index];
}