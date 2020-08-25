#include "constraint_declaration.hpp"

//SELF
#include "../constraints/constraint.hpp"

using namespace element;

constraint_declaration::constraint_declaration(identifier name, const scope* parent_scope, const bool is_intrinsic)
    : declaration(std::move(name), parent_scope)
    , constraint_(std::make_unique<constraint>(4, this)) //todo: what to use
{
    qualifier = constraint_qualifier;
    _intrinsic = is_intrinsic;
}

bool constraint_declaration::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    return constraint_->matches_constraint(context, constraint);
}

const constraint* constraint_declaration::get_constraint() const
{
    if (is_intrinsic())
        return constraint::any.get();

    return constraint_.get();
}