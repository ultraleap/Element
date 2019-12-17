#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <cstdlib>

#include "element/interpreter.h"
#include "ast/ast_internal.hpp"
#include "ast/fwd.hpp"
#include "construct.hpp"
#include "ast/scope.hpp"


struct element_type_constraint : public element_construct
{
public:
    static const type_constraint_const_shared_ptr any;
    static const type_constraint_const_shared_ptr function;
    static const type_constraint_const_shared_ptr serializable;

    // TODO: separate into "shape matches" and "type matches"
    virtual bool is_satisfied_by(const type_constraint_const_shared_ptr& other) const
    {
        return other.get() == this || other == element_type_constraint::any;
    }

protected:
    element_type_constraint() = default;
};


struct element_type : public element_type_constraint
{
public:
    static const type_const_shared_ptr num;  // the absolute unit
    static const type_const_shared_ptr unary;
    static const type_const_shared_ptr binary;

    // TODO: make this accurate?
    virtual bool is_serializable() const { return get_size() != 0; }
    virtual bool is_variadic() const { return false; }

    std::string name() const { return m_name; }

protected:
    element_type(std::string name)
        : element_type_constraint()
        , m_name(std::move(name))
    {
    }

    std::string m_name;
};

struct element_custom_type : public element_type
{
    element_custom_type(const element_scope* scope)
        : element_type(scope->name)
        , m_scope(scope)
    {
    }

    const element_scope* scope() const { return m_scope; }

protected:
    void generate_ports_cache() const override;

    const element_scope* m_scope;
};

struct element_anonymous_type : public element_type
{
    element_anonymous_type(std::vector<port_info> inputs, std::vector<port_info> outputs)
        : element_type("<anonymous>")
    {
        m_inputs = std::move(inputs);
        m_outputs = std::move(outputs);
        m_ports_cached = true;
    }
};
