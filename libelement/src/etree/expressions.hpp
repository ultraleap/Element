#pragma once

#include <string>
#include <vector>
#include <utility>
#include <numeric>
#include <unordered_map>

#include "element/interpreter.h"
#include "ast/ast_internal.hpp"
#include "ast/fwd.hpp"
#include "etree/fwd.hpp"
#include "construct.hpp"
#include "typeutil.hpp"

struct element_expression : public element_construct, public rtti_type<element_expression>
{
    const std::vector<expression_shared_ptr>& dependents() const { return m_dependents; }
    std::vector<expression_shared_ptr>& dependents() { return m_dependents; }

protected:
    element_expression(element_type_id t)
        : element_construct()
        , rtti_type(t)
    {
        // expressions have no inputs/outputs, so leave them empty
        m_ports_cached = true;
    }

    std::vector<expression_shared_ptr> m_dependents;
    element_type_id m_type_id = 0;
};

struct element_expression_constant : public element_expression
{
    DECLARE_TYPE_ID();

    element_expression_constant(element_value val)
        : element_expression(type_id)
        , m_value(val)
    {
    }

    element_value value() const { return m_value; }
    size_t get_size() const override { return 1; }
private:
    element_value m_value;
};

//User-provided input from the boundary
struct element_expression_input : public element_expression
{
    DECLARE_TYPE_ID();

    element_expression_input(size_t input_index, size_t input_size)
        : element_expression(type_id)
        , m_index(input_index)
        , m_size(input_size)
    {
    }

    size_t index() const { return m_index; }
    size_t get_size() const override { return m_size; }
private:
    size_t m_index;
    size_t m_size;
};

struct element_expression_structure : public element_expression
{
    DECLARE_TYPE_ID();

    element_expression_structure(std::vector<std::pair<std::string, expression_shared_ptr>> deps)
        : element_expression(type_id)
    {
        m_dependents.reserve(deps.size());
        for (const auto& d : deps) {
            m_dependents.emplace_back(d.second);
            m_dependents_map[d.first] = d.second;
        }
    }

    expression_shared_ptr output(const std::string& name) const
    {
        auto it = m_dependents_map.find(name);
        return (it != m_dependents_map.end()) ? it->second : nullptr;
    }

    size_t get_size() const override
    {
        return std::accumulate(m_dependents.begin(), m_dependents.end(), size_t(0),
            [](size_t c, const auto& d) { return c + d->get_size(); });
    }

private:
    std::unordered_map<std::string, expression_shared_ptr> m_dependents_map;
};

struct element_expression_unary : public element_expression
{
    DECLARE_TYPE_ID();

    using op = element_unary_op;

    element_expression_unary(op t, expression_shared_ptr input)
        : element_expression(type_id)
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(input));
    }

    op operation() const { return m_op; }
    const expression_shared_ptr& input() const { return m_dependents[0]; }

    size_t get_size() const override { return 1; }
private:
    op m_op;
};

struct element_expression_binary : public element_expression
{
    DECLARE_TYPE_ID();

    using op = element_binary_op;

    element_expression_binary(op t, expression_shared_ptr in1, expression_shared_ptr in2)
        : element_expression(type_id)
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(in1));
        m_dependents.emplace_back(std::move(in2));
    }

    op operation() const { return m_op; }
    const expression_shared_ptr& input1() const { return m_dependents[0]; }
    const expression_shared_ptr& input2() const { return m_dependents[1]; }

    size_t get_size() const override { return 1; }
private:
    op m_op;
};

//
// Expression groups
//
struct element_expr_group : public element_expression
{
    DECLARE_TYPE_ID();

    // TODO: implement
protected:
    element_expr_group()
        : element_expression(type_id)
    {
    }

    // virtual size_t group_size() const = 0;
};

struct element_unbound_arg : public element_expression
{
    DECLARE_TYPE_ID();
    
    element_unbound_arg(size_t idx)
        : element_expression(type_id)
        , m_index(idx)
    {
    }

    size_t index() const { return m_index; }

protected:
    size_t m_index;
};