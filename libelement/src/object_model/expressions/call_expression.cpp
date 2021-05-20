#include "call_expression.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "expression_chain.hpp"
#include "object_model/object_internal.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/declarations/declaration.hpp"
#include "instruction_tree/instructions.hpp"

using namespace element;

call_expression::call_expression(const expression_chain* parent)
    : expression(parent)
{
}

[[nodiscard]] object_const_shared_ptr call_expression::resolve(const compilation_context& context, const object* obj)
{
    std::vector<object_const_shared_ptr> compiled_arguments;
    compiled_arguments.reserve(arguments.size());

    for (const auto& arg : arguments)
        compiled_arguments.push_back(arg->compile(context, source_info));

    for (const auto& arg : compiled_arguments)
    {
        if (arg->is_error())
            return arg;
    }

    auto element = obj->call(context, std::move(compiled_arguments), source_info);
    if (element)
        return element;

    //note: we assume that the error will be contained in the returned object, and thus should never be null.
    //errors should be built where they occur, to make debugging easier and to ensure we have access to all required information
    assert(!"internal compiler error");
    return build_error_and_log<error_message_code::invalid_errorless_call>(context, source_info, parent->declarer->name.value);
}