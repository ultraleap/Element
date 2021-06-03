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

        if (!our_input.is_valid(context))
            return true; //allow constraint matching for invalid ports, to propagate errors and catch multiple

        if (!their_input.is_valid(context))
            return true; //allow constraint matching for invalid ports, to propagate errors and catch multiple

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

    //todo: nothing should currently use the any constraint, not even intrinsic any, it's all nullptr
    if (!constraint_ || constraint_ == any.get())
        return true;

    //todo: if there is no declarer it's num or bool (types), which can't match a function constraint
    const auto* other = constraint_->declarer;
    if (!other)
        return false;

    std::size_t our_input_length = declarer->inputs.size();
    std::size_t offset = 0;

    if (applied)
    {
        our_input_length--;
        offset++;
    }

    if (our_input_length != other->get_inputs().size())
        return false;

    for (std::size_t i = 0; i < our_input_length; ++i)
        if (!check_match(declarer->inputs[i + offset], other->inputs[i]))
            return false;

    //allow constraint matching for invalid ports, to propagate errors and catch multiple
    const bool our_annotation_is_invalid = !declarer->output.is_valid(context);
    const bool their_annotation_is_invalid = !other->output.is_valid(context);

    if (our_annotation_is_invalid || their_annotation_is_invalid)
        return true;

    //todo: nullptr checks?
    if (!check_match(declarer->output, other->output))
        return false;

    return true;
}