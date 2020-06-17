#ifndef LEGACY_COMPILER

#include "functions.hpp"
#include "types.hpp"
#include "intermediaries.hpp"
#include "etree/expressions.hpp"

DEFINE_TYPE_ID(element::intrinsic_nullary, 1U << 0);
DEFINE_TYPE_ID(element::intrinsic_unary, 1U << 1);
DEFINE_TYPE_ID(element::intrinsic_binary, 1U << 2);
//DEFINE_TYPE_ID(element::intrinsic_if, 1U << 3);

namespace element
{
    const std::unordered_map<std::string, const std::shared_ptr<const intrinsic>> intrinsic::intrinsic_map
    {
        //types
        { "Num", nullptr },
        { "Bool", nullptr },
        { "List", nullptr },
        { "Tuple", nullptr },

        //functions
        { "Num.add", std::make_shared<intrinsic_binary>(element_binary_op::add) },
        { "Num.sub", std::make_shared<intrinsic_binary>(element_binary_op::sub) },
        { "Num.mul", std::make_shared<intrinsic_binary>(element_binary_op::mul) },
        { "Num.div", std::make_shared<intrinsic_binary>(element_binary_op::div) },

        { "Num.pow", std::make_shared<intrinsic_binary>(element_binary_op::pow) },
        { "Num.rem", std::make_shared<intrinsic_binary>(element_binary_op::rem) },

        { "Num.min", std::make_shared<intrinsic_binary>(element_binary_op::min) },
        { "Num.max", std::make_shared<intrinsic_binary>(element_binary_op::max) },

        { "Num.abs", std::make_shared<intrinsic_unary>(element_unary_op::abs) },
        { "Num.ceil", std::make_shared<intrinsic_unary>(element_unary_op::ceil) },
        { "Num.floor", std::make_shared<intrinsic_unary>(element_unary_op::floor) },

        { "Num.sin", std::make_shared<intrinsic_unary>(element_unary_op::sin) },
        { "Num.cos", std::make_shared<intrinsic_unary>(element_unary_op::cos) },
        { "Num.tan", std::make_shared<intrinsic_unary>(element_unary_op::tan) },

        { "Num.asin", std::make_shared<intrinsic_unary>(element_unary_op::asin) },
        { "Num.acos", std::make_shared<intrinsic_unary>(element_unary_op::acos) },
        { "Num.atan", std::make_shared<intrinsic_unary>(element_unary_op::atan) },

        { "Num.atan2", std::make_shared<intrinsic_binary>(element_binary_op::atan2) },

        { "Num.ln", std::make_shared<intrinsic_unary>(element_unary_op::ln) },
        { "Num.log", std::make_shared<intrinsic_binary>(element_binary_op::log) },

        { "Num.NaN", std::make_shared<intrinsic_nullary>(element_nullary_op::nan) },
        { "Num.PositiveInfinity", std::make_shared<intrinsic_nullary>(element_nullary_op::positive_infinity) },
        { "Num.NegativeInfinity", std::make_shared<intrinsic_nullary>(element_nullary_op::negative_infinity) },

        { "Num.eq", std::make_shared<intrinsic_binary>(element_binary_op::eq, element_type::boolean, element_type::num, element_type::num) },
        { "Num.neq", std::make_shared<intrinsic_binary>(element_binary_op::neq, element_type::boolean, element_type::num, element_type::num) },
        { "Num.lt", std::make_shared<intrinsic_binary>(element_binary_op::lt, element_type::boolean, element_type::num, element_type::num) },
        { "Num.leq", std::make_shared<intrinsic_binary>(element_binary_op::leq, element_type::boolean, element_type::num, element_type::num) },
        { "Num.gt", std::make_shared<intrinsic_binary>(element_binary_op::gt, element_type::boolean, element_type::num, element_type::num) },
        { "Num.geq", std::make_shared<intrinsic_binary>(element_binary_op::geq, element_type::boolean, element_type::num, element_type::num) },

        { "Bool.if", nullptr },
        { "Bool.not", std::make_shared<intrinsic_unary>(element_unary_op::not, element_type::boolean, element_type::boolean) },
        { "Bool.and", std::make_shared<intrinsic_binary>(element_binary_op::and, element_type::boolean, element_type::boolean) },
        { "Bool.or", std::make_shared<intrinsic_binary>(element_binary_op::or , element_type::boolean, element_type::boolean) },

        { "True", std::make_shared<intrinsic_nullary>(element_nullary_op::true_value, element_type::boolean) },
        { "False", std::make_shared<intrinsic_nullary>(element_nullary_op::false_value, element_type::boolean) },

        { "list", nullptr },
        { "List.fold", nullptr },

        { "memberwise", nullptr },
        { "for", nullptr },
        //{ "persist", nullptr }, //todo: unlikely to be part of the language

        //constraints
        { "Any", nullptr },
    };

    std::shared_ptr<const intrinsic> intrinsic::get_intrinsic(const declaration& declaration)
    {
        const auto location = declaration.location();

        const auto it = intrinsic_map.find(location);
        if (it != intrinsic_map.end())
            return it->second;

        return nullptr;
    }

    intrinsic::intrinsic(element_type_id id, type_const_shared_ptr type)
        : rtti_type(id), return_type(std::move(type))
    {
    }

    intrinsic_nullary::intrinsic_nullary(element_nullary_op operation, type_const_shared_ptr return_type = element_type::num):
        intrinsic(type_id, std::move(return_type))
        , operation(operation)
    {
    }

    std::shared_ptr<object> intrinsic_nullary::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        auto result = std::make_shared<compiled_expression>();
        result->creator = nullptr;

        result->expression_tree = std::make_shared<element_expression_nullary>(
            operation);

        return result;
    }

    intrinsic_unary::intrinsic_unary(element_unary_op operation, 
                                     type_const_shared_ptr return_type = element_type::num, 
                                     type_const_shared_ptr argument_type = element_type::num)
        : intrinsic(type_id, std::move(return_type))
        , operation(operation)
        , argument_type{std::move(argument_type)}
    {
    }

    std::shared_ptr<object> intrinsic_unary::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        auto result = std::make_shared<compiled_expression>();
        result->creator = nullptr;

        result->expression_tree = std::make_shared<element_expression_unary>(
            operation,
            args[0]->expression_tree);

        return result;
    }

    intrinsic_binary::intrinsic_binary(element_binary_op operation, type_const_shared_ptr return_type = element_type::num,
                                       type_const_shared_ptr first_argument_type = element_type::num,
                                       type_const_shared_ptr second_argument_type = element_type::num):
        intrinsic(type_id, std::move(return_type))
        , operation(operation)
        , first_argument_type{std::move(first_argument_type)}
        , second_argument_type{std::move(second_argument_type)}
    {
    }

    std::shared_ptr<object> intrinsic_binary::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        auto result = std::make_shared<compiled_expression>();
        result->creator = nullptr;

        result->expression_tree = std::make_shared<element_expression_binary>(
            operation,
            args[0]->expression_tree,
            args[1]->expression_tree);

        return result;
    }
}


#else

#include "ast/functions.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include <unordered_map>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"

DEFINE_TYPE_ID(element_intrinsic,                1U << 0);
DEFINE_TYPE_ID(element_intrinsic_nullary,        1U << 1);
DEFINE_TYPE_ID(element_intrinsic_unary,          1U << 2);
DEFINE_TYPE_ID(element_intrinsic_binary,         1U << 3);
DEFINE_TYPE_ID(element_type_ctor,                1U << 4);
DEFINE_TYPE_ID(element_custom_function,          1U << 5);
DEFINE_TYPE_ID(element_intrinsic_if,             1U << 6);

#define MAKE_UNARY(name)  { #name, std::make_shared<element_intrinsic_unary>(element_expression_unary::op::name, element_type::unary, #name) }
#define MAKE_BINARY(name) { #name, std::make_shared<element_intrinsic_binary>(element_expression_binary::op::name, element_type::binary, #name) }
#define MAKE_UNARY_BOOLEAN(name) { #name, std::make_shared<element_intrinsic_unary>(element_expression_unary::op::name, element_type::unary_boolean, #name) }
#define MAKE_BINARY_BOOLEAN(name) { #name, std::make_shared<element_intrinsic_binary>(element_expression_binary::op::name, element_type::binary_boolean, #name) }
#define MAKE_COMPARISON_BOOLEAN(name) { #name, std::make_shared<element_intrinsic_binary>(element_expression_binary::op::name, element_type::binary_comparison, #name) }

function_const_shared_ptr element_function::get_builtin(const std::string& name)
{
    // Important: this must NOT be made as part of normal static initialisation, as it depends on other static objects
    static std::unordered_map<std::string, function_const_shared_ptr> builtins{
        // types
        { "Num",              std::make_shared<element_type_ctor>(element_type::num) },
        { "Bool",             std::make_shared<element_type_ctor>(element_type::boolean) },
    	
        //{ "List",           std::make_shared<element_intrinsic_nullary>(element_expression_nullary::op::positive_infinity, element_type::nullary, "List") },

        // special functions
        { "if",               std::make_shared<element_intrinsic_if>(element_type::if_condition, "if") },
        //{ "for",            std::make_shared<element_intrinsic_for>() },
        //{ "fold",           std::make_shared<element_intrinsic_fold>() },
        //{ "infer",          std::make_shared<element_intrinsic_infer>() },
        //{ "memberwise",     std::make_shared<element_intrinsic_memberwise>() },

    	// nullary functions
        { "True",             std::make_shared<element_intrinsic_nullary>(element_expression_nullary::op::true_value, element_type::nullary_boolean, "True") },
        { "False",            std::make_shared<element_intrinsic_nullary>(element_expression_nullary::op::false_value, element_type::nullary_boolean, "False") },
        { "NaN",              std::make_shared<element_intrinsic_nullary>(element_expression_nullary::op::nan, element_type::nullary, "NaN") },
        { "PositiveInfinity", std::make_shared<element_intrinsic_nullary>(element_expression_nullary::op::positive_infinity, element_type::nullary, "PositiveInfinity") },
        { "NegativeInfinity", std::make_shared<element_intrinsic_nullary>(element_expression_nullary::op::negative_infinity, element_type::nullary, "NegativeInfinity") },
   
    	// functions
        MAKE_BINARY(add),
        MAKE_BINARY(sub),
        MAKE_BINARY(mul),
        MAKE_BINARY(div),
    	
        MAKE_BINARY(pow),
        MAKE_BINARY(rem),
    	
        MAKE_BINARY(min),
        MAKE_BINARY(max),

        MAKE_UNARY(abs),
        MAKE_UNARY(ceil),
        MAKE_UNARY(floor),

        MAKE_UNARY(sin),
        MAKE_UNARY(cos),
        MAKE_UNARY(tan),

        MAKE_UNARY(asin),
        MAKE_UNARY (acos),
        MAKE_UNARY (atan),
        MAKE_BINARY(atan2),
    	
        MAKE_UNARY(ln),
        MAKE_BINARY(log),

        MAKE_UNARY_BOOLEAN(not),
        MAKE_BINARY_BOOLEAN(and),
        MAKE_BINARY_BOOLEAN(or),

        MAKE_COMPARISON_BOOLEAN(eq),
        MAKE_COMPARISON_BOOLEAN(neq),
        MAKE_COMPARISON_BOOLEAN(lt),
        MAKE_COMPARISON_BOOLEAN(leq),
        MAKE_COMPARISON_BOOLEAN(gt),
        MAKE_COMPARISON_BOOLEAN(geq),
    };

    auto it = builtins.find(name);
    return (it != builtins.end()) ? it->second : nullptr;
}
#undef MAKE_UNARY
#undef MAKE_BINARY

void element_function::generate_ports_cache() const
{
    m_inputs = m_type->inputs();
    m_outputs = m_type->outputs();
    m_ports_cached = true;
}


type_shared_ptr element_custom_function::generate_type(const element_scope* scope) const
{
    std::vector<port_info> inputs;
    std::vector<port_info> outputs;

    const element_ast* node = scope->node;
    assert(ast_node_function_is_valid(node));

    if (node->type == ELEMENT_AST_NODE_FUNCTION) {
        const element_ast* declaration = ast_node_function_get_declaration(node);
        assert(ast_node_declaration_has_inputs(declaration));

        if (ast_node_declaration_get_inputs(declaration)->type == ELEMENT_AST_NODE_PORTLIST)
            inputs = generate_portlist(scope, declaration->children[ast_idx::declaration::inputs].get());

        if (ast_node_declaration_has_outputs(declaration)) {
	        const auto* out = ast_node_declaration_get_outputs(declaration);

            if (out->type == ELEMENT_AST_NODE_TYPENAME)
                outputs.push_back({ "return", find_typename(scope, out) });
            else if (out->type == ELEMENT_AST_NODE_UNSPECIFIED_TYPE)  // any
                outputs.push_back({ "return", element_constraint::any });

            //todo: what if it's not?
        }
    }

    return element_type_anonymous::get(std::move(inputs), std::move(outputs));
}

#endif
