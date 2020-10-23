#include "user_function_constraint.hpp"

//SELF
#include "object_model/declarations/declaration.hpp"

using namespace element;

bool user_function_constraint::matches_constraint(const compilation_context& context, const constraint* constraint_) const
{
    const auto check_match = [&context](const port& our_input, const port& their_input) {
        const declaration* our_input_type = nullptr;
        const declaration* their_input_type = nullptr;
        const constraint* our_input_constraint = nullptr;
        const constraint* their_input_constraint = nullptr;

        if (our_input.has_annotation())
            our_input_type = our_input.resolve_annotation(context);

        if (their_input.has_annotation())
            their_input_type = their_input.resolve_annotation(context);

        if (our_input_type)
            our_input_constraint = our_input_type->get_constraint();

        if (their_input_type)
            their_input_constraint = their_input_type->get_constraint();

        if (!their_input_constraint || their_input_constraint == any.get())
            return true;

        if (our_input_constraint != their_input_constraint)
            return false;

        //even if the pointers match, we need to call matches_constraint to make sure
        return our_input_constraint->matches_constraint(context, their_input_constraint);
    };

    if (!constraint_ || constraint_ == any.get())
        return true;

    //todo: if there is no declarer it's num or bool (types), which can't match a function constraint
    const auto* other = constraint_->declarer;
    if (!other)
        return false;

    for (unsigned i = 0; i < declarer->inputs.size(); ++i)
    {
        const auto& our_input = declarer->inputs[i];
        const auto& their_input = other->inputs[i];

        if (!check_match(our_input, their_input))
            return false;
    }

    //no one has a return constraint
    if (!declarer->output && !other->output)
        return true;

    //check return types match since at least one of them has one
    if (declarer->output && other->output)
    {
        //todo: nullptr checks?
        if (!check_match(declarer->output.value(), other->output.value()))
            return false;
    }

    return true;
}