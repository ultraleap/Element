#include "intrinsic_compiler_list_indexer.hpp"

//STD
#include <algorithm>

//SELF
#include "object_model/intermediaries/list_element_wrapper.hpp"
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

    return list_element_wrapper::create_or_optimise(our_arguments[0], list_arguments, source_info);
}