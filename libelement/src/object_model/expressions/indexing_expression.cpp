#include "indexing_expression.hpp"

//SELF
#include "object_model/object_internal.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"

using namespace element;

indexing_expression::indexing_expression(identifier name, const expression_chain* parent)
    : expression(parent)
    , name(std::move(name))
{
    assert(parent);
}

[[nodiscard]] object_const_shared_ptr indexing_expression::resolve(const compilation_context& context, const object* obj)
{
    auto element = obj->index(context, name, source_info);
    if (element)
        return element;

    return build_error_and_log(context, source_info, error_message_code::failed_to_find_when_resolving_indexing_expr, name.value, obj->typeof_info());
}