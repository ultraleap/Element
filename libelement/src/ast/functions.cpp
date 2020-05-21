#include "ast/functions.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include <unordered_map>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"

DEFINE_TYPE_ID(element_intrinsic,        1U << 0);
DEFINE_TYPE_ID(element_intrinsic_unary,  1U << 1);
DEFINE_TYPE_ID(element_intrinsic_binary, 1U << 2);
DEFINE_TYPE_ID(element_type_ctor,        1U << 3);
DEFINE_TYPE_ID(element_custom_function,  1U << 4);

#define MAKE_UNARY(name)  { #name, std::make_shared<element_intrinsic_unary>(element_expression_unary::op::name, #name) }
#define MAKE_BINARY(name) { #name, std::make_shared<element_intrinsic_binary>(element_expression_binary::op::name, #name) }
function_const_shared_ptr element_function::get_builtin(const std::string& name)
{
    // Important: this must NOT be made as part of normal static initialisation, as it depends on other static objects
    static std::unordered_map<std::string, function_const_shared_ptr> builtins{
        // types
        { "Num",   std::make_shared<element_type_ctor>(element_type::num)                },
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
        const element_ast* decl = ast_node_function_get_declaration(node);
        assert(ast_node_declaration_has_inputs(decl));

        if (ast_node_declaration_get_inputs(decl)->type == ELEMENT_AST_NODE_PORTLIST)
            inputs = generate_portlist(scope, decl->children[ast_idx::decl::inputs].get());

        if (ast_node_declaration_has_outputs(decl)) {
            const element_ast* out = ast_node_declaration_get_outputs(decl);

            if (out->type == ELEMENT_AST_NODE_TYPENAME)
                outputs.push_back({ "return", find_typename(scope, out) });
            else if (out->type == ELEMENT_AST_NODE_NONE)  // any
                outputs.push_back({ "return", element_constraint::any });

            //todo: what if it's not?
        }
    }

    return element_type_anonymous::get(std::move(inputs), std::move(outputs));
}