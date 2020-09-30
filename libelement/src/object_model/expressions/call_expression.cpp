#include "call_expression.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "expression_chain.hpp"
#include "object_model/object.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/declarations/declaration.hpp"
#include "etree/expressions.hpp"

using namespace element;

call_expression::call_expression(const expression_chain* parent)
    : expression(parent)
{
    assert(parent);
}

[[nodiscard]] object_const_shared_ptr call_expression::resolve(const compilation_context& context, const object* obj)
{
    std::vector<object_const_shared_ptr> compiled_arguments;
    for (const auto& arg : arguments)
        compiled_arguments.push_back(arg->compile(context, source_info));

    for (const auto& arg : compiled_arguments)
    {
        const auto* err = dynamic_cast<const error*>(arg.get());
        if (err)
            return arg;
    }

    auto element = obj->call(context, std::move(compiled_arguments), source_info);
    if (element)
        return element;

    assert(!"internal compiler error");
    return build_error_and_log(context, source_info, error_message_code::invalid_errorless_call, parent->declarer->name.value);
}