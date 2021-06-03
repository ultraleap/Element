#include "intrinsic.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "intrinsic_constructor_bool.hpp"
#include "intrinsic_constructor_num.hpp"
#include "intrinsic_nullary.hpp"
#include "intrinsic_unary.hpp"
#include "intrinsic_binary.hpp"
#include "intrinsic_if.hpp"
#include "intrinsic_list.hpp"
#include "intrinsic_list_fold.hpp"
#include "intrinsic_for.hpp"
#include "intrinsic_compiler_list_indexer.hpp"
#include "intrinsic_constructor_list.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/constraints/type.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/declarations/constraint_declaration.hpp"
#include "object_model/declarations/struct_declaration.hpp"
#include "instruction_tree/instructions.hpp"
#include "instruction_tree/evaluator.hpp"

using namespace element;

DEFINE_TYPE_ID(element::intrinsic_nullary, 1U << 0);
DEFINE_TYPE_ID(element::intrinsic_unary, 1U << 1);
DEFINE_TYPE_ID(element::intrinsic_binary, 1U << 2);
DEFINE_TYPE_ID(element::intrinsic_if, 1U << 3);
DEFINE_TYPE_ID(element::intrinsic_constructor_num, 1U << 4);
DEFINE_TYPE_ID(element::intrinsic_constructor_bool, 1U << 5);
DEFINE_TYPE_ID(element::intrinsic_constructor_list, 1U << 6);
DEFINE_TYPE_ID(element::intrinsic_list, 1U << 7);
DEFINE_TYPE_ID(element::intrinsic_compiler_list_indexer, 1U << 8);
DEFINE_TYPE_ID(element::intrinsic_for, 1U << 9);
DEFINE_TYPE_ID(element::intrinsic_list_fold, 1U << 10);

template <typename T>
static bool is_type_of(const declaration* decl)
{
    return dynamic_cast<const T*>(decl) ? true : false;
}

[[nodiscard]] std::string intrinsic::get_name() const
{
    return ""; //todo: the map has all the names...
}

//TODO: This is a horrible, temporary hack. Remove and replace once we start dealing with constraints
const std::unordered_map<std::string, std::function<std::unique_ptr<const intrinsic>(const declaration*)>> intrinsic::validation_func_map{
    //type
    { "Num", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? std::make_unique<intrinsic_constructor_num>() : nullptr); } },
    { "Bool", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? std::make_unique<intrinsic_constructor_bool>() : nullptr); } },
    { "List", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? std::make_unique<intrinsic_constructor_list>() : nullptr); } },
    { "Tuple", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? std::make_unique<intrinsic_not_implemented>() : nullptr); } },

    //functions
    { "add", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::add, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "sub", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::sub, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "mul", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::mul, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "div", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::div, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "pow", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::pow, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "rem", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::rem, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "min", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::min, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "max", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::max, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "abs", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::abs, type::num.get(), type::num.get()) : nullptr); } },
    { "ceil", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::ceil, type::num.get(), type::num.get()) : nullptr); } },
    { "floor", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::floor, type::num.get(), type::num.get()) : nullptr); } },

    { "sin", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::sin, type::num.get(), type::num.get()) : nullptr); } },
    { "cos", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::cos, type::num.get(), type::num.get()) : nullptr); } },
    { "tan", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::tan, type::num.get(), type::num.get()) : nullptr); } },

    { "asin", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::asin, type::num.get(), type::num.get()) : nullptr); } },
    { "acos", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::acos, type::num.get(), type::num.get()) : nullptr); } },
    { "atan", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::atan, type::num.get(), type::num.get()) : nullptr); } },

    { "atan2", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::atan2, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "ln", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::ln, type::num.get(), type::num.get()) : nullptr); } },
    { "log", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::log, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "NaN", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_nullary>(element_nullary_op::nan, type::num.get()) : nullptr); } },
    { "PositiveInfinity", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_nullary>(element_nullary_op::positive_infinity, type::num.get()) : nullptr); } },
    { "NegativeInfinity", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_nullary>(element_nullary_op::negative_infinity, type::num.get()) : nullptr); } },

    { "eq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::eq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "neq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::neq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "lt", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::lt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "leq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::leq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "gt", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::gt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "geq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::geq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "if", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_if>() : nullptr); } },
    { "not", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_unary>(element_unary_op::not_, type::boolean.get(), type::boolean.get()) : nullptr); } },
    { "and", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::and_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },
    { "or", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_binary>(element_binary_op::or_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },

    { "True", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_nullary>(element_nullary_op::true_value, type::boolean.get()) : nullptr); } },
    { "False", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_nullary>(element_nullary_op::false_value, type::boolean.get()) : nullptr); } },

    { "list", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_list>() : nullptr); } },
    { "fold", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_list_fold>() : nullptr); } },

    { "for", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_for>() : nullptr); } },

    //constraints
    { "Any", [](const declaration* decl) { return (is_type_of<constraint_declaration>(decl) ? std::make_unique<intrinsic_not_implemented>() : nullptr); } },

    //compiler-implemented hidden thingies that aren't intrinsic but kinda are
    { "@list_indexer", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? std::make_unique<intrinsic_compiler_list_indexer>() : nullptr); } },
};

intrinsic::intrinsic(const element_type_id id)
    : rtti_type(id)
{
}

std::pair<std::vector<object_const_shared_ptr>, size_t> element::generate_placeholder_inputs(
    const compilation_context& compilation_context,
    const std::vector<port>& inputs,
    std::size_t index_offset,
    const std::size_t boundary_scope)
{
    std::pair<std::vector<object_const_shared_ptr>, size_t> placeholder_inputs;

    for (const auto& input : inputs)
    {
        auto placeholder = input.generate_placeholder(compilation_context, index_offset, boundary_scope);
        placeholder_inputs.first.push_back(std::move(placeholder));
    }

    placeholder_inputs.second = index_offset;
    return placeholder_inputs;
}

std::shared_ptr<const instruction> element::evaluate(const compilation_context& context, instruction_const_shared_ptr expr)
{
    float output = 0;
    std::size_t output_count = 1;
    
    element_evaluator_ctx evaluator;
    const auto result = element_evaluate(evaluator, expr, nullptr, nullptr, 0, &output, output_count);

    //the tree was fully evaluated, so it has been constant folded
    if (result == ELEMENT_OK)
    {
        auto new_expr = std::make_shared<const instruction_constant>(output);
        new_expr->actual_type = expr->actual_type;
        return new_expr;
    }
    
    //we failed to fully evaluate the tree, likely due to boundary inputs (whose value are not known), so try and optimise it differently

    if (const auto* binary = expr->as<const instruction_binary>())
    {
        auto optimised = optimise_binary(*binary);
        if (optimised)
            return optimised;
    }

    if (const auto* selector = expr->as<const instruction_select>())
    {
        //if there's only one option to pick from then we're guaranteed to pick it, so we can just treat it as that option
        if (selector->options_count() == 1)
            return selector->options_at(0);
    }

    //todo: optimise other instructions

    //we couldn't optimise it, so just return the original tree
    return expr;
}
