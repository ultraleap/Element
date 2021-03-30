#pragma once

//SELF
#include "intrinsic.hpp"
#include "object_model/compilation_context.hpp"
#include "instruction_tree/evaluator.hpp"

namespace element
{
    class intrinsic_function : public intrinsic
    {
    public:
        intrinsic_function(element_type_id id, type_const_ptr return_type, bool variadic = false);
        [[nodiscard]] type_const_ptr get_type() const final { return return_type; };
        [[nodiscard]] bool is_variadic() const { return variadic; }

    protected:
        //TODO: this might need to be a constraint_const_shared_ptr
        type_const_ptr return_type;
        bool variadic = false;
    };

    //todo: move somewhere else
    static std::shared_ptr<const instruction> evaluate(const compilation_context& context, instruction_const_shared_ptr expr)
    {
        float output = 0;
        std::size_t output_count = 1;
        
        element_evaluator_ctx evaluator;
        const auto result = element_evaluate(evaluator, expr, nullptr, 0, &output, output_count);
        if (result != ELEMENT_OK)
            return expr;

        auto new_expr = std::make_shared<const instruction_constant>(output);
        new_expr->actual_type = expr->actual_type;
        return new_expr;
    }
} // namespace element