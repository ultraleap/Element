#include "types.hpp"

#ifndef LEGACY_COMPILER

namespace element
{
    DEFINE_TYPE_ID(element_type, 1U << 0);
    DEFINE_TYPE_ID(num_type, 1U << 1);
    DEFINE_TYPE_ID(boolean_type, 1U << 2);

    DEFINE_TYPE_ID(any_constraint, 1U << 3);
    DEFINE_TYPE_ID(function_constraint, 1U << 4);
    DEFINE_TYPE_ID(nullary_constraint, 1U << 5);
    DEFINE_TYPE_ID(unary_constraint, 1U << 6);
    DEFINE_TYPE_ID(binary_constraint, 1U << 7);

    const type_const_shared_ptr element_type::num = std::make_shared<num_type>();
    const type_const_shared_ptr element_type::boolean = std::make_shared<boolean_type>();

    const constraint_const_shared_ptr element_constraint::any = std::make_shared<any_constraint>();
    const constraint_const_shared_ptr element_constraint::function = std::make_shared<function_constraint>();
    const constraint_const_shared_ptr element_constraint::nullary = std::make_shared<nullary_constraint>();
    const constraint_const_shared_ptr element_constraint::unary = std::make_shared<unary_constraint>();
    const constraint_const_shared_ptr element_constraint::binary = std::make_shared<binary_constraint>();
}

#else

#include "ast/types.hpp"

#include <functional>
#include <cassert>
#include "interpreter_internal.hpp"
#include "ast/ast_indexes.hpp"

std::unordered_multimap<std::pair<size_t, size_t>, type_shared_ptr, pair_hash> element_type_anonymous::m_cache;


struct any_constraint : public element_constraint
{
    DECLARE_TYPE_ID();
    any_constraint() : element_constraint(type_id) {}
    bool is_satisfied_by(const constraint_const_shared_ptr& value) const override { return true; }
};

struct function_constraint : public element_constraint
{
    DECLARE_TYPE_ID();
    function_constraint() : element_constraint(type_id) {}
    bool is_satisfied_by(const constraint_const_shared_ptr& value) const override { return !value->inputs().empty(); }
};

struct serializable_constraint : public element_constraint
{
    DECLARE_TYPE_ID();
    serializable_constraint() : element_constraint(type_id) {}
    // TODO: this, if we still need it
    bool is_satisfied_by(const constraint_const_shared_ptr& value) const override { return false; }
};

const constraint_const_shared_ptr element_constraint::any = std::make_shared<any_constraint>();
const constraint_const_shared_ptr element_constraint::function = std::make_shared<function_constraint>();
const constraint_const_shared_ptr element_constraint::serializable = std::make_shared<serializable_constraint>();


// TODO: interpreter-specific, can't be static!
static const element_scope* num_scope = nullptr;
static const element_scope* bool_scope = nullptr;
// the prelude includes a definition for num, so it should be based on custom_type (and updated later)
struct num_type : public element_type_named
{
    num_type() : element_type_named(nullptr, "Num") { m_ports_cached = true; }
    size_t get_size() const override { return 1; }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override { return v->inputs().empty() && v->outputs().empty(); }

    const element_scope* scope() const override { return num_scope; }
protected:
    void generate_ports_cache() const override {}
};

struct boolean_type : public element_type_named {
    boolean_type() : element_type_named(nullptr, "Bool") { m_ports_cached = true; }
    size_t get_size() const override { return 1; }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override { return v->inputs().empty() && v->outputs().empty(); }

    const element_scope* scope() const override { return bool_scope; }
protected:
    void generate_ports_cache() const override {}
};

void update_scopes(const element_scope* names)
{
    num_scope = names->lookup("Num", false);
    bool_scope = names->lookup("Bool", false);
}

struct nullary_type : public element_type {
    DECLARE_TYPE_ID();

    nullary_type() : element_type(type_id, "<nullary>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
    {
        return v->inputs().empty()
	        && v->outputs().size() == 1 && v->outputs()[0].type == element_type::num;
    }
protected:
    void generate_ports_cache() const override
    {
        m_outputs.push_back({ "return", element_type::num });
        m_ports_cached = true;
    }
};

struct unary_type : public element_type
{
    DECLARE_TYPE_ID();

    unary_type() : element_type(type_id, "<unary>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
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
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
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

struct nullary_boolean_type : public element_type{
    DECLARE_TYPE_ID();

    nullary_boolean_type() : element_type(type_id, "<nullary boolean>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
    {
        return v->inputs().empty()
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::boolean;
    }
protected:
    void generate_ports_cache() const override
    {
        m_outputs.push_back({ "return", element_type::boolean });
        m_ports_cached = true;
    }
};

struct unary_boolean_type : public element_type {
    DECLARE_TYPE_ID();

    unary_boolean_type() : element_type(type_id, "<unary boolean>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
    {
        return v->inputs().size() == 1 && v->inputs()[0].type == element_type::boolean
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::boolean;
    }
protected:
    void generate_ports_cache() const override
    {
        m_inputs.push_back({ "a", element_type::boolean });
        m_outputs.push_back({ "return", element_type::boolean });
        m_ports_cached = true;
    }
};

struct binary_boolean_type : public element_type {
    DECLARE_TYPE_ID();

    binary_boolean_type() : element_type(type_id, "<binary boolean>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
    {
        return v->inputs().size() == 2 && v->inputs()[0].type == element_type::boolean && v->inputs()[1].type == element_type::boolean
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::boolean;
    }
protected:
    void generate_ports_cache() const override
    {
        m_inputs.push_back(port_info{ "a", element_type::boolean });
        m_inputs.push_back(port_info{ "b", element_type::boolean });
        m_outputs.push_back(port_info{ "return", element_type::boolean });
        m_ports_cached = true;
    }
};

struct binary_comparison_type : public element_type {
    DECLARE_TYPE_ID();

    binary_comparison_type() : element_type(type_id, "<binary boolean>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
    {
        return v->inputs().size() == 2 && v->inputs()[0].type == element_type::num && v->inputs()[1].type == element_type::num
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::boolean;
    }
protected:
    void generate_ports_cache() const override
    {
        m_inputs.push_back(port_info{ "a", element_type::num });
        m_inputs.push_back(port_info{ "b", element_type::num });
        m_outputs.push_back(port_info{ "return", element_type::boolean });
        m_ports_cached = true;
    }
};

//TODO: Needs to be handled via list with dynamic indexing, this will be insufficient for when we have user input
struct if_condition_type : public element_type {
    DECLARE_TYPE_ID();

    if_condition_type() : element_type(type_id, "<if>") { generate_ports_cache(); }
    bool is_serializable() const override { return true; } // TODO
    bool is_satisfied_by(const constraint_const_shared_ptr& v) const override
    {
        return v->inputs().size() == 3 && v->inputs()[0].type == element_type::boolean && v->inputs()[1].type == element_type::any && v->inputs()[2].type == element_type::any
            && v->outputs().size() == 1 && v->outputs()[0].type == element_type::any;
    }
protected:
    void generate_ports_cache() const override
    {
        m_inputs.push_back(port_info{ "predicate", element_type::boolean });
        m_inputs.push_back(port_info{ "if_true", element_type::any });
        m_inputs.push_back(port_info{ "if_false", element_type::any });
        m_outputs.push_back(port_info{ "return", element_type::any });
        m_ports_cached = true;
    }
};

DEFINE_TYPE_ID(element_type,            1U << 0);
DEFINE_TYPE_ID(element_type_named,      1U << 1);
DEFINE_TYPE_ID(element_type_anonymous,  1U << 2);
DEFINE_TYPE_ID(nullary_type,            1U << 3);
DEFINE_TYPE_ID(unary_type,              1U << 4);
DEFINE_TYPE_ID(binary_type,             1U << 5);
DEFINE_TYPE_ID(nullary_boolean_type,    1U << 6);
DEFINE_TYPE_ID(unary_boolean_type,      1U << 7);
DEFINE_TYPE_ID(binary_boolean_type,     1U << 8);
DEFINE_TYPE_ID(binary_comparison_type,  1U << 9);
DEFINE_TYPE_ID(any_constraint,          1U << 10);
DEFINE_TYPE_ID(function_constraint,     1U << 11);
DEFINE_TYPE_ID(serializable_constraint, 1U << 12);
DEFINE_TYPE_ID(if_condition_type,       1U << 13);

const type_const_shared_ptr element_type::num = std::make_shared<num_type>();
const type_const_shared_ptr element_type::boolean = std::make_shared<boolean_type>();
const type_const_shared_ptr element_type::nullary = std::make_shared<nullary_type>();
const type_const_shared_ptr element_type::unary = std::make_shared<unary_type>();
const type_const_shared_ptr element_type::binary = std::make_shared<binary_type>();
const type_const_shared_ptr element_type::nullary_boolean = std::make_shared<nullary_boolean_type>();
const type_const_shared_ptr element_type::unary_boolean = std::make_shared<unary_boolean_type>();
const type_const_shared_ptr element_type::binary_boolean = std::make_shared<binary_boolean_type>();
const type_const_shared_ptr element_type::binary_comparison = std::make_shared<binary_comparison_type>();
const type_const_shared_ptr element_type::if_condition = std::make_shared<if_condition_type>();


void element_type_named::generate_ports_cache() const
{
    const element_ast* node = m_scope->node;
    assert(node);

    //todo: when is a named type not a struct?
    if (node->type != ELEMENT_AST_NODE_STRUCT)
        return;

    if (ast_node_struct_has_body(node)) {
        for (const auto& [child_name, child_scope] : m_scope->children) {
            //todo: a bit of a hack. a structs "return" scope is there so it can be used with compile_custom_fn_scope I guess? but does it also need to be part of the types output?
            const auto child_function = child_scope->function();
            m_outputs.emplace_back(port_info{ child_name, child_function ? child_function->type() : nullptr }); //TODO: CPP20 - emplace_back can use aggregate initialization
        }
    }

    const element_ast* declaration = ast_node_struct_get_declaration(node);
    if (ast_node_declaration_has_inputs(declaration)) {
	    const auto inputs = ast_node_declaration_get_inputs(declaration);

        if (inputs->type == ELEMENT_AST_NODE_PORTLIST) {
            m_inputs = generate_portlist(m_scope, inputs);
        }
    }

    if (ast_node_declaration_has_outputs(declaration)) {
	    const auto outputs = ast_node_declaration_get_outputs(declaration);

        if (outputs->type == ELEMENT_AST_NODE_PORTLIST) {
            m_outputs = generate_portlist(m_scope, outputs); //todo: is this valid? does it work?
        } else if (outputs->type == ELEMENT_AST_NODE_TYPENAME) {
            m_outputs.emplace_back(port_info{ "return", find_typename(m_scope, outputs) }); //TODO: CPP20 - emplace_back can use aggregate initialization
        }
    }

    m_ports_cached = true;
}

#endif