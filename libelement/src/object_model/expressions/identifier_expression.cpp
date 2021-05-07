#include "identifier_expression.hpp"

//SELF
#include "expression_chain.hpp"
#include "object_model/declarations/declaration.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/compilation_context.hpp"

using namespace element;

identifier_expression::identifier_expression(identifier name, const expression_chain* parent)
    : expression(parent)
    , name(std::move(name))
{
    assert(parent);
}

[[nodiscard]] object_const_shared_ptr identifier_expression::resolve(const compilation_context& context, const object* obj)
{
    auto element = context.captures.find(parent->declarer->our_scope.get(), name, context, parent->source_info);
    if (element)
        return element;

    return build_error_and_log<error_message_code::failed_to_find_when_resolving_identifier_expr>(context, source_info, name.value);
}