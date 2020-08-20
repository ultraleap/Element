#include "types.hpp"

//STD
#include <string>

//SELF
#include "scope.hpp"

namespace element
{
    DEFINE_TYPE_ID(any_constraint, 1U << 1);
    DEFINE_TYPE_ID(user_function_constraint, 1U << 2);
    DEFINE_TYPE_ID(type, 1U << 16);
    DEFINE_TYPE_ID(num_type, 1U << 17);
    DEFINE_TYPE_ID(boolean_type, 1U << 18);
    DEFINE_TYPE_ID(user_type, 1U << 19);

    identifier num_type::name{ "Num" };
    identifier boolean_type::name{ "Bool" };

    const constraint_const_unique_ptr constraint::any = std::make_unique<any_constraint>();
    const type_const_unique_ptr type::num = std::make_unique<num_type>();
    const type_const_unique_ptr type::boolean = std::make_unique<boolean_type>();

    object_const_shared_ptr num_type::index(const compilation_context& context, const identifier& name,
                                                  const source_information& source_info) const
    {
        //todo: cache, but don't use static
        const auto declaration = context.get_global_scope()->find(num_type::name, false);
        assert(declaration);
        return declaration->index(context, name, source_info);
    }

    object_const_shared_ptr boolean_type::index(const compilation_context& context, const identifier& name,
                                                      const source_information& source_info) const
    {
        //todo: cache, but don't use static
        const auto declaration = context.get_global_scope()->find(boolean_type::name, false);
        assert(declaration);
        return declaration->index(context, name, source_info);
    }

    bool constraint::matches_constraint(const compilation_context& context, const constraint* constraint) const
    {
        if (!constraint || constraint == any.get())
            return true;

        return this == constraint;
    }

    bool user_function_constraint::matches_constraint(const compilation_context& context, const constraint* constraint_) const
    {
        const auto check_match = [&context](const port& our_input, const port& their_input)
        {
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
}