#include "anonymous_block_instance.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "etree/expressions.hpp"
#include "object_model/expressions/anonymous_block_expression.hpp"
#include "object_model/constraints/constraint.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/compilation_context.hpp"

using namespace element;

anonymous_block_instance::anonymous_block_instance(const anonymous_block_expression* declarer, capture_stack captures, source_information source_info)
    : declarer(declarer)
    , captures(std::move(captures))
{
    this->source_info = std::move(source_info);
}

bool element::anonymous_block_instance::is_constant() const
{
    return true;
}

bool anonymous_block_instance::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    //todo: might need to do more, see below, but for now it can be treated as any
    return constraint == constraint::any.get() || !constraint;
}

const constraint* anonymous_block_instance::get_constraint() const
{
    //todo: we need a constraint representing anonymous blocks but unsure what that looks like, do they all share the same constraint, or each instance is unique, or the same if the scope & capture is the same?
    return nullptr;
}

object_const_shared_ptr anonymous_block_instance::index(const compilation_context& context,
                                                        const identifier& name,
                                                        const source_information& source_info) const
{
    std::swap(captures, context.captures);
    auto found = context.captures.find(declarer->our_scope.get(), name, context, declarer->source_info);
    std::swap(captures, context.captures);
    if (found)
        return found;

    return build_error_and_log(context, source_info, error_message_code::failed_to_find, name.value);
}

object_const_shared_ptr anonymous_block_instance::compile(const compilation_context& context,
                                                          const source_information& source_info) const
{
    return shared_from_this();
}