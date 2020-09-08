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
#include "etree/expressions.hpp"
#include "etree/evaluator.hpp"

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

template<typename T>
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
const std::unordered_map<std::string, std::function<std::unique_ptr<const intrinsic, element_interpreter_ctx::Deleter>(const declaration*)>> intrinsic::validation_func_map
{
    //type
    { "Num", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_constructor_num>() : nullptr); } },
    { "Bool", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_constructor_bool>() : nullptr); } },
    { "List", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_constructor_list>() : nullptr); } },
    { "Tuple", [](const declaration* decl) { return (is_type_of<struct_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },

    //functions
    { "Num.add", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::add, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.sub", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::sub, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.mul", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::mul, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.div", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::div, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "Num.pow", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::pow, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.rem", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::rem, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "Num.min", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::min, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.max", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::max, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "Num.abs",   [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::abs, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.ceil",  [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::ceil, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.floor", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::floor, type::num.get(), type::num.get()) : nullptr); } },

    { "Num.sin", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::sin, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.cos", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::cos, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.tan", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::tan, type::num.get(), type::num.get()) : nullptr); } },

    { "Num.asin", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::asin, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.acos", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::acos, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.atan", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::atan, type::num.get(), type::num.get()) : nullptr); } },

    { "Num.atan2", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::atan2, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "Num.ln", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::ln, type::num.get(), type::num.get()) : nullptr); } },
    { "Num.log", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::log, type::num.get(), type::num.get(), type::num.get()) : nullptr); } },

    { "Num.NaN", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::nan, type::num.get()) : nullptr); } },
    { "Num.PositiveInfinity", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::positive_infinity, type::num.get()) : nullptr); } },
    { "Num.NegativeInfinity", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::negative_infinity, type::num.get()) : nullptr); } },

    { "Num.eq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::eq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.neq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::neq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.lt", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::lt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.leq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::leq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.gt", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::gt, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    { "Num.geq", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::geq, type::boolean.get(), type::num.get(), type::num.get()) : nullptr); } },
    
    { "Bool.if", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_if>() : nullptr); } },
    { "Bool.not", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_unary>(element_unary_op::not_, type::boolean.get(), type::boolean.get()) : nullptr); } },
    { "Bool.and", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::and_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },
    { "Bool.or", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_binary>(element_binary_op::or_, type::boolean.get(), type::boolean.get(), type::boolean.get()) : nullptr); } },

    { "True", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::true_value, type::boolean.get()) : nullptr); } },
    { "False", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_nullary>(element_nullary_op::false_value, type::boolean.get()) : nullptr); } },

    { "list", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_list>() : nullptr); } },
    { "List.fold", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_list_fold> () : nullptr); } },

    { "memberwise", [](const declaration* decl) { return (is_type_of<function_declaration>(decl) ? make_unique<intrinsic_not_implemented>() : nullptr); } },
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
    element_result& out_result,
    const int index_offset)
{
    std::pair<std::vector<object_const_shared_ptr>, size_t> placeholder_inputs;
    auto placeholder_index = index_offset;

    for (const auto& input : inputs)
    {
        auto placeholder = input.generate_placeholder(compilation_context, placeholder_index);
        if (!placeholder)
            out_result = ELEMENT_ERROR_UNKNOWN;

        placeholder_inputs.first.push_back(std::move(placeholder));
    }

    placeholder_inputs.second = placeholder_index;
    return placeholder_inputs;
}