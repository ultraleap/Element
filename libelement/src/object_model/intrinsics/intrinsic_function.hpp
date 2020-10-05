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

    static std::shared_ptr<const element_instruction> evaluate(const compilation_context& context, std::shared_ptr<const element_instruction> expr)
    {
        std::vector<element_value> outputs = { 0 };
        const auto result = element_evaluate(*context.interpreter, expr, {}, outputs, {});
        if (result != ELEMENT_OK)
            return expr;

        auto new_expr = std::make_unique<element_instruction_constant>(outputs[0]);
        new_expr->actual_type = expr->actual_type;
        return new_expr;
    }
} // namespace element