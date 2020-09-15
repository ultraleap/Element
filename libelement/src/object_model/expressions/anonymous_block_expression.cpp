#include "anonymous_block_expression.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "expression_chain.hpp"
#include "object_model/object.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/intermediaries/anonymous_block_instance.hpp"
#include "etree/expressions.hpp"

using namespace element;

anonymous_block_expression::anonymous_block_expression(const expression_chain* parent)
    : expression(parent)
{
    assert(parent);
}

object_const_shared_ptr anonymous_block_expression::resolve(const compilation_context& context, const object* obj)
{
    if (obj)
        return std::make_shared<const error>("anonymous block isn't the first thing in the chain", ELEMENT_ERROR_UNKNOWN, source_info);

    return std::make_shared<const anonymous_block_instance>(this, context.captures, source_info);
}