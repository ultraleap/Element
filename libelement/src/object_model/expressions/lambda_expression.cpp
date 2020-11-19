#include "lambda_expression.hpp"

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

lambda_expression::lambda_expression(const expression_chain* parent)
    : expression(parent)
{
}

[[nodiscard]] object_const_shared_ptr lambda_expression::resolve(const compilation_context& context, const object* obj)
{
    assert(!obj);
    return function->compile(context, source_info);
}