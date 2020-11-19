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
#include "intrinsic_memberwise.hpp"
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
DEFINE_TYPE_ID(element::intrinsic_memberwise, 1U << 11);

template <typename T>
static bool is_type_of(const declaration* decl)
{
    return dynamic_cast<const T*>(decl) ? true : false;
}

template <typename Class, typename... Args>
std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter> make_unique(Args&&... args)
{
    static_assert(std::is_base_of_v<intrinsic, Class>, "must be derived from intrinsic");
    return std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(static_cast<const intrinsic*>(new Class(std::forward<Args>(args)...)));
}

template <typename Class>
std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter> make_unique()
{
    static_assert(std::is_base_of_v<intrinsic, Class>, "must be derived from intrinsic");
    return std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(static_cast<const intrinsic*>(new Class()));
}

//TODO: This is a horrible, temporary hack. Remove and replace once we start dealing with constraints
const std::unordered_map<std::string, std::function<std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(const declaration*)>> intrinsic::validation_func_map{
    //type
    { "Num", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_constructor_num>() : nullptr); } },
    { "Bool", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_constructor_bool>() : nullptr); } },
    { "List", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_constructor_list>() : nullptr); } },
    { "Tuple", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

    //functions
    { "add", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::add, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "sub", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::sub, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "mul", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::mul, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "div", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::div, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "pow", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::pow, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "rem", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::rem, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "min", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::min, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "max", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::max, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "abs", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::abs, type::num.get(), type::num.get()) : nullptr); } },
    { "ceil", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::ceil, type::num.get(), type::num.get()) : nullptr); } },
    { "floor", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::floor, type::num.get(), type::num.get()) : nullptr); } },

    { "sin", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::sin, type::num.get(), type::num.get()) : nullptr); } },
    { "cos", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::cos, type::num.get(), type::num.get()) : nullptr); } },
    { "tan", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::tan, type::num.get(), type::num.get()) : nullptr); } },

    { "asin", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::asin, type::num.get(), type::num.get()) : nullptr); } },
    { "acos", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::acos, type::num.get(), type::num.get()) : nullptr); } },
    { "atan", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::atan, type::num.get(), type::num.get()) : nullptr); } },

    { "atan2", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::atan2, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "ln", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::ln, type::num.get(), type::num.get()) : nullptr); } },
    { "log", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::log, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "NaN", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::nan, type::num.get()) : nullptr); } },
    { "PositiveInfinity", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::positive_infinity, type::num.get()) : nullptr); } },
    { "NegativeInfinity", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::negative_infinity, type::num.get()) : nullptr); } },

    { "eq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::eq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "neq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::neq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "lt", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::lt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "leq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::leq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "gt", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::gt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "geq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::geq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "if", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_if>() : nullptr); } },
    { "not", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::not_, type::boolean.get(), type::boolean.get()) : nullptr); } },
    { "and", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::and_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },
    { "or", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::or_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },

    { "True", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::true_value, type::boolean.get()) : nullptr); } },
    { "False", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::false_value, type::boolean.get()) : nullptr); } },

    { "list", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_list>() : nullptr); } },
    { "fold", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_list_fold>() : nullptr); } },

    { "memberwise", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_memberwise>() : nullptr); } },
    { "for", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_for>() : nullptr); } },
    { "persist", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

    //constraints
    { "Any", [](const declaration* decl) { return (is_type_of<constraint_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

    //compiler-implemented hidden thingies that aren't intrinsic but kinda are
    { "@list_indexer", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_compiler_list_indexer>() : nullptr); } },
};

intrinsic::intrinsic(const element_type_id id)
    : rtti_type(id)
{
}

std::pair<std::vector<object_const_shared_ptr>, size_t> element::generate_placeholder_inputs(
    const compilation_context& compilation_context,
    const std::vector<port>& inputs,
    const int index_offset,
    const unsigned int boundary_scope)
{
    std::pair<std::vector<object_const_shared_ptr>, size_t> placeholder_inputs;
    auto placeholder_index = index_offset;

    for (const auto& input : inputs)
    {
        auto placeholder = input.generate_placeholder(compilation_context, placeholder_index, boundary_scope);
        placeholder_inputs.first.push_back(std::move(placeholder));
    }

    placeholder_inputs.second = placeholder_index;
    return placeholder_inputs;
}