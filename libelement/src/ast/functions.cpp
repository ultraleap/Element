#include "ast/functions.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include <unordered_map>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"

DEFINE_TYPE_ID(element_intrinsic,        1U << 0);
DEFINE_TYPE_ID(element_unary_intrinsic,  1U << 1);
DEFINE_TYPE_ID(element_binary_intrinsic, 1U << 2);
DEFINE_TYPE_ID(element_type_ctor,        1U << 3);
DEFINE_TYPE_ID(element_custom_function,  1U << 4);

#define MAKE_UNARY(name)  { #name, std::make_shared<element_unary_intrinsic>(element_unary::op::name, #name) }
#define MAKE_BINARY(name) { #name, std::make_shared<element_binary_intrinsic>(element_binary::op::name, #name) }
function_const_shared_ptr element_function::get_builtin(const std::string& name)
{
    // Important: this must NOT be made as part of normal static initialisation, as it depends on other static objects
    static std::unordered_map<std::string, function_const_shared_ptr> builtins{
        // types
        { "num",   std::make_shared<element_type_ctor>(element_type::num)                },
        // functions
        MAKE_BINARY(add),
        MAKE_UNARY (acos),
        MAKE_UNARY (asin),
        MAKE_UNARY (atan),
        MAKE_BINARY(atan2),
        MAKE_UNARY (ceil),
        MAKE_UNARY (cos),
        MAKE_BINARY(div),
        MAKE_UNARY (floor),
        MAKE_UNARY (ln),
        MAKE_BINARY(log),
        MAKE_BINARY(max),
        MAKE_BINARY(min),
        MAKE_BINARY(mul),
        MAKE_BINARY(pow),
        MAKE_BINARY(rem),
        MAKE_UNARY (sin),
        MAKE_BINARY(sub),
        MAKE_UNARY (tan),
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


type_shared_ptr element_custom_function::generate_type(const element_scope* scope)
{
    std::vector<port_info> inputs;
    std::vector<port_info> outputs;

    const element_ast* node = scope->node;
    assert(node->type == ELEMENT_AST_NODE_FUNCTION);
    if (node->type == ELEMENT_AST_NODE_FUNCTION) {
        assert(node->children.size() > ast_idx::fn::declaration);
        element_ast* decl = node->children[ast_idx::fn::declaration].get();
        assert(decl->type == ELEMENT_AST_NODE_DECLARATION);
        assert(decl->children.size() > ast_idx::decl::inputs);
        if (decl->children[ast_idx::decl::inputs]->type == ELEMENT_AST_NODE_PORTLIST) {
            inputs = generate_portlist(scope, decl->children[ast_idx::decl::inputs].get());
        }
        if (decl->children.size() > ast_idx::decl::outputs) {
            if (decl->children[ast_idx::decl::outputs]->type == ELEMENT_AST_NODE_TYPENAME) {
                outputs.push_back({ "return", find_typename(scope, decl->children[ast_idx::decl::outputs].get()) });
            } else if (decl->children[ast_idx::decl::outputs]->type == ELEMENT_AST_NODE_NONE) {  // any
                outputs.push_back({ "return", element_type_constraint::any });
            }
        }
    }

    return element_anonymous_type::get(std::move(inputs), std::move(outputs));
}