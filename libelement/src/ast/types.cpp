#include "ast/types.hpp"

#include <algorithm>
#include <functional>
#include <cassert>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"


std::unordered_multimap<std::pair<size_t, size_t>, type_shared_ptr, pair_hash> element_anonymous_type::m_cache;


struct any_constraint : public element_type_constraint
{
    bool is_satisfied_by(const type_constraint_const_shared_ptr& value) const override { return true; }
};

struct function_constraint : public element_type_constraint
{
    bool is_satisfied_by(const type_constraint_const_shared_ptr& value) const override { return !value->inputs().empty(); }
};

struct serializable_constraint : public element_type_constraint
{
    // TODO: this, if we still need it
    bool is_satisfied_by(const type_constraint_const_shared_ptr& value) const override { return false; }
};

const type_constraint_const_shared_ptr element_type_constraint::any = std::make_shared<any_constraint>();
const type_constraint_const_shared_ptr element_type_constraint::function = std::make_shared<function_constraint>();
const type_constraint_const_shared_ptr element_type_constraint::serializable = std::make_shared<serializable_constraint>();


struct num_type : public element_type
{
    num_type() : element_type("num") { m_ports_cached = true; }
    size_t get_size() const override { return 1; }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const type_constraint_const_shared_ptr& v) const override { return v->inputs().empty() && v->outputs().empty(); }
protected:
    void generate_ports_cache() const override {}
};

struct unary_type : public element_type
{
    unary_type() : element_type("<unary>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const type_constraint_const_shared_ptr& v) const override
    {
        return v->inputs().size() == 1 && v->inputs()[0].type == element_type::num
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::num;
    }
protected:
    void generate_ports_cache() const override
    {
        m_inputs.push_back({"a", element_type::num});
        m_outputs.push_back({"return", element_type::num});
        m_ports_cached = true;
    }
};

struct binary_type : public element_type
{
    binary_type() : element_type("<binary>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const type_constraint_const_shared_ptr& v) const override
    {
        return v->inputs().size() == 2 && v->inputs()[0].type == element_type::num && v->inputs()[1].type == element_type::num
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::num;
    }
protected:
    void generate_ports_cache() const override
    {
        m_inputs.push_back(port_info{"a", element_type::num});
        m_inputs.push_back(port_info{"b", element_type::num});
        m_outputs.push_back(port_info{"return", element_type::num});
        m_ports_cached = true;
    }
};

const type_const_shared_ptr element_type::num = std::make_shared<num_type>();
const type_const_shared_ptr element_type::unary = std::make_shared<unary_type>();
const type_const_shared_ptr element_type::binary = std::make_shared<binary_type>();


void element_custom_type::generate_ports_cache() const
{
    const element_ast* node = m_scope->node;
    if (node->type == ELEMENT_AST_NODE_STRUCT) {
        assert(node->children.size() > ast_idx::fn::declaration);
        element_ast* decl = node->children[ast_idx::fn::declaration].get();
        assert(decl->type == ELEMENT_AST_NODE_DECLARATION);
        if (decl->children.size() > ast_idx::decl::outputs) {
            if (decl->children[ast_idx::decl::outputs]->type == ELEMENT_AST_NODE_PORTLIST) {
                m_outputs = generate_portlist(m_scope, decl->children[ast_idx::decl::outputs].get());
            } else if (decl->children[ast_idx::decl::outputs]->type == ELEMENT_AST_NODE_TYPENAME) {
                m_outputs.push_back({ "return", find_typename(m_scope, decl->children[ast_idx::decl::outputs].get()) });
            }
        }
        if (node->children.size() > ast_idx::fn::body) {
            assert(node->children[ast_idx::fn::body]->type == ELEMENT_AST_NODE_SCOPE);
            for (auto& kv : m_scope->children) {
                auto fn = kv.second->function();
                m_outputs.push_back({ kv.first, fn ? fn->type() : nullptr });
            }
        }
        m_ports_cached = true;
    }
}