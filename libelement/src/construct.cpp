#include "construct.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"


type_const_shared_ptr element_construct::find_typename(const element_scope* scope, element_ast* node) const
{
    assert(node->type == ELEMENT_AST_NODE_TYPENAME);
    std::vector<std::string> names;
    for (auto& child : node->children) {
        assert(child->type == ELEMENT_AST_NODE_IDENTIFIER);
        names.push_back(child->identifier);
    }
    const element_scope* result = scope->lookup(names);
    return (result && result->function()) ? result->function()->type() : nullptr;
}

std::vector<port_info> element_construct::generate_portlist(const element_scope* scope, element_ast* portlist) const
{
    assert(portlist->type == ELEMENT_AST_NODE_PORTLIST);
    std::vector<port_info> result;
    for (auto& child : portlist->children) {
        assert(child->type == ELEMENT_AST_NODE_PORT);
        std::string child_id = child->identifier;
        type_constraint_const_shared_ptr child_type = element_type_constraint::any;
        if (child->children.size() > ast_idx::port::type) {
            child_type = find_typename(scope, child->children[ast_idx::port::type].get());
        }
        result.push_back({ child_id, child_type });
    }
    return result;
}

const std::vector<port_info>& element_construct::inputs() const
{
    if (!m_ports_cached)
        generate_ports_cache();
    return m_inputs;
}

const std::vector<port_info>& element_construct::outputs() const
{
    if (!m_ports_cached)
        generate_ports_cache();
    return m_outputs;
}

size_t element_construct::get_size() const
{
    size_t sz = 0;
    for (const auto& output : outputs())
        sz += output.type->get_size();
    return sz;
}
