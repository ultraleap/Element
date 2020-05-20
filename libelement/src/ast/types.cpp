#include "ast/types.hpp"

#include <functional>
#include <cassert>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"

std::unordered_multimap<std::pair<size_t, size_t>, type_shared_ptr, pair_hash> element_anonymous_type::m_cache;


struct any_constraint : public element_type_constraint
{
    DECLARE_TYPE_ID();
    any_constraint() : element_type_constraint(type_id) {}
    bool is_satisfied_by(const type_constraint_const_shared_ptr& value) const override { return true; }
};

struct function_constraint : public element_type_constraint
{
    DECLARE_TYPE_ID();
    function_constraint() : element_type_constraint(type_id) {}
    bool is_satisfied_by(const type_constraint_const_shared_ptr& value) const override { return !value->inputs().empty(); }
};

struct serializable_constraint : public element_type_constraint
{
    DECLARE_TYPE_ID();
    serializable_constraint() : element_type_constraint(type_id) {}
    // TODO: this, if we still need it
    bool is_satisfied_by(const type_constraint_const_shared_ptr& value) const override { return false; }
};

const type_constraint_const_shared_ptr element_type_constraint::any = std::make_shared<any_constraint>();
const type_constraint_const_shared_ptr element_type_constraint::function = std::make_shared<function_constraint>();
const type_constraint_const_shared_ptr element_type_constraint::serializable = std::make_shared<serializable_constraint>();


// TODO: interpreter-specific, can't be static!
static const element_scope* num_scope = nullptr;
// the prelude includes a definition for num, so it should be based on custom_type (and updated later)
struct num_type : public element_custom_type
{
    num_type() : element_custom_type(nullptr, "Num") { m_ports_cached = true; }
    size_t get_size() const override { return 1; }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const type_constraint_const_shared_ptr& v) const override { return v->inputs().empty() && v->outputs().empty(); }

    const element_scope* scope() const override { return num_scope; }
protected:
    void generate_ports_cache() const override {}
};

void update_scopes(const element_scope* names)
{
    num_scope = names->lookup("Num", false);
}


struct unary_type : public element_type
{
    DECLARE_TYPE_ID();

    unary_type() : element_type(type_id, "<unary>") { generate_ports_cache(); }
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
    DECLARE_TYPE_ID();

    binary_type() : element_type(type_id, "<binary>") { generate_ports_cache(); }
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

DEFINE_TYPE_ID(element_type,           1U << 0);
DEFINE_TYPE_ID(element_custom_type,    1U << 1);
DEFINE_TYPE_ID(element_anonymous_type, 1U << 2);
DEFINE_TYPE_ID(unary_type,             1U << 3);
DEFINE_TYPE_ID(binary_type,            1U << 4);
DEFINE_TYPE_ID(any_constraint,         1U << 5);
DEFINE_TYPE_ID(function_constraint,    1U << 6);
DEFINE_TYPE_ID(serializable_constraint,1U << 7);

const type_const_shared_ptr element_type::num = std::make_shared<num_type>();
const type_const_shared_ptr element_type::unary = std::make_shared<unary_type>();
const type_const_shared_ptr element_type::binary = std::make_shared<binary_type>();


void element_custom_type::generate_ports_cache() const
{
    const element_ast* node = m_scope->node;
    assert(node);

    //todo: when is a type not a struct?
    if (node->type != ELEMENT_AST_NODE_STRUCT)
        return;

    if (ast_node_struct_has_body(node)) {
        //todo: do structs without scoped bodies, still have a child? do they generate this node?
        assert(ast_node_struct_get_body(node)->type == ELEMENT_AST_NODE_SCOPE);

        for (auto& [child_name, child_scope] : m_scope->children) {
            const auto child_function = child_scope->function();
            m_outputs.emplace_back(port_info{ child_name, child_function ? child_function->type() : nullptr }); //TODO: CPP20 - emplace_back can use aggregate initialization
        }
    }

    const element_ast* decl = ast_node_struct_get_declaration(node);
    if (ast_node_declaration_has_outputs(decl)) {
        const element_ast* outputs = ast_node_declaration_get_outputs(decl);

        if (outputs->type == ELEMENT_AST_NODE_PORTLIST) {
            m_outputs = generate_portlist(m_scope, outputs);
        } else if (outputs->type == ELEMENT_AST_NODE_TYPENAME) {
            m_outputs.emplace_back(port_info{ "return", find_typename(m_scope, outputs) }); //TODO: CPP20 - emplace_back can use aggregate initialization
        }
    }

    m_ports_cached = true;
}