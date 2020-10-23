#include "constraint.hpp"

//SELF
#include "object_model/compilation_context.hpp"
#include "any_constraint.hpp"
#include "type.hpp"
#include "bool_type.hpp"
#include "num_type.hpp"
#include "user_type.hpp"
#include "user_function_constraint.hpp"

using namespace element;

DEFINE_TYPE_ID(any_constraint, 1U << 1);
DEFINE_TYPE_ID(user_function_constraint, 1U << 2);
DEFINE_TYPE_ID(type, 1U << 16);
DEFINE_TYPE_ID(num_type, 1U << 17);
DEFINE_TYPE_ID(bool_type, 1U << 18);
DEFINE_TYPE_ID(user_type, 1U << 19);

identifier num_type::name{ "Num" };
identifier bool_type::name{ "Bool" };

const constraint_const_unique_ptr constraint::any = std::make_unique<any_constraint>();
const type_const_unique_ptr type::num = std::make_unique<num_type>();
const type_const_unique_ptr type::boolean = std::make_unique<bool_type>();

bool constraint::matches_constraint(const compilation_context& context, const constraint* constraint) const
{
    if (!constraint || constraint == any.get())
        return true;

    return this == constraint;
}