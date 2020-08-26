#pragma once

#include <string>
#include <vector>
#include <utility>
#include <numeric>
#include <unordered_map>

#include "../ast/ast_internal.hpp"
#include "../ast/fwd.hpp"
#include "etree/fwd.hpp"
#include "typeutil.hpp"
#include "object_model/constraints/type.hpp"
#include "object_model/object.hpp"

struct element_expression : element::object, rtti_type<element_expression>, std::enable_shared_from_this<element_expression>
{
public:
    [[nodiscard]] const std::vector<expression_const_shared_ptr>& dependents() const { return m_dependents; }
    std::vector<expression_const_shared_ptr>& dependents() { return m_dependents; }

    [[nodiscard]] virtual size_t get_size() const { return m_size; }

    [[nodiscard]] bool matches_constraint(const element::compilation_context& context, const element::constraint* constraint) const final { return actual_type ? actual_type->matches_constraint(context, constraint) : false; };
    [[nodiscard]] const element::constraint* get_constraint() const final { return actual_type; };

    [[nodiscard]] std::string typeof_info() const override;
    [[nodiscard]] std::string to_code(int depth = 0) const override;

    [[nodiscard]] std::shared_ptr<const element_expression> to_expression() const final { return shared_from_this(); };

    //todo: this is changed by the num constructor, but it should actually copy the expression with the new type
    mutable element::type_const_ptr actual_type;

protected:
    explicit element_expression(element_type_id t, element::type_const_ptr actual_type)
        : rtti_type(t)
        , actual_type(std::move(actual_type))
    {
    }

    std::vector<expression_const_shared_ptr> m_dependents;
    element_type_id m_type_id = 0;
    int m_size = 0;

    [[nodiscard]] element::object_const_shared_ptr compile(
        const element::compilation_context& context,
        const element::source_information& source_info) const override;

    [[nodiscard]] element::object_const_shared_ptr index(
        const element::compilation_context& context,
        const element::identifier& name,
        const element::source_information& source_info) const override;
};

struct element_expression_constant final : public element_expression
{
    DECLARE_TYPE_ID();

    explicit element_expression_constant(element_value val);

    [[nodiscard]] element_value value() const { return m_value; }
    [[nodiscard]] size_t get_size() const override { return 1; }
private:
    element_value m_value;
};

//User-provided input from the boundary
struct element_expression_input final : public element_expression
{
    DECLARE_TYPE_ID();

    explicit element_expression_input(size_t input_index)
        : element_expression(type_id, nullptr)
        , m_index(input_index)
    {
    }

    [[nodiscard]] size_t index() const { return m_index; }
    [[nodiscard]] size_t get_size() const override { return 1; }

private:
    size_t m_index;
};

struct element_expression_structure final : public element_expression
{
    DECLARE_TYPE_ID();

    explicit element_expression_structure(const std::vector<std::pair<std::string, expression_const_shared_ptr>>& deps)
        : element_expression(type_id, nullptr)
    {
        m_dependents.reserve(deps.size());
        for (const auto& [name, expression] : deps) {
            m_dependents.emplace_back(expression);
            m_dependents_map[name] = expression;
        }
    }

    [[nodiscard]] expression_const_shared_ptr output(const std::string& name) const
    {
        const auto it = m_dependents_map.find(name);
        return (it != m_dependents_map.end()) ? it->second : nullptr;
    }

    [[nodiscard]] size_t get_size() const override
    {
        return std::accumulate(m_dependents.begin(), m_dependents.end(), size_t(0),
            [](size_t c, const auto& d) { return c + d->get_size(); });
    }

private:
    std::unordered_map<std::string, expression_const_shared_ptr> m_dependents_map;
};

struct element_expression_nullary final : public element_expression
{
    DECLARE_TYPE_ID();

    using op = element_nullary_op;

    explicit element_expression_nullary(op t, element::type_const_ptr actual_type)
        : element_expression(type_id, std::move(actual_type))
        , m_op(t)
    {
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] size_t get_size() const override { return 1; }
private:
    op m_op;
};

//TODO: NEED TO PASS RETURN TYPE HERE
struct element_expression_unary final : public element_expression
{
    DECLARE_TYPE_ID();

    using op = element_unary_op;

    explicit element_expression_unary(op t, expression_const_shared_ptr input, element::type_const_ptr actual_type)
        : element_expression(type_id, std::move(actual_type))
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(input));
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] const expression_const_shared_ptr& input() const { return m_dependents[0]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
private:
    op m_op;
};

//TODO: NEED TO PASS RETURN TYPE HERE
struct element_expression_binary final : public element_expression
{
    DECLARE_TYPE_ID();

    using op = element_binary_op;

    explicit element_expression_binary(op t, expression_const_shared_ptr in1, expression_const_shared_ptr in2, element::type_const_ptr actual_type)
        : element_expression(type_id, std::move(actual_type))
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(in1));
        m_dependents.emplace_back(std::move(in2));
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] const expression_const_shared_ptr& input1() const { return m_dependents[0]; }
    [[nodiscard]] const expression_const_shared_ptr& input2() const { return m_dependents[1]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
private:
    op m_op;
};

struct element_expression_if final : public element_expression
{
    DECLARE_TYPE_ID();

    explicit element_expression_if(expression_const_shared_ptr predicate, expression_const_shared_ptr if_true, expression_const_shared_ptr if_false);
    [[nodiscard]] const expression_const_shared_ptr& predicate() const { return m_dependents[0]; }
    [[nodiscard]] const expression_const_shared_ptr& if_true() const { return m_dependents[1]; }
    [[nodiscard]] const expression_const_shared_ptr& if_false() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
};

////
//// Expression groups
////
//struct element_expression_group : public element_expression
//{
//    DECLARE_TYPE_ID();
//
//    //todo: do we still need expression groups?
//protected:
//    element_expression_group()
//        : element_expression(type_id)
//    {
//    }
//
//    // virtual size_t group_size() const = 0;
//};
//
//struct element_expression_unbound_arg : public element_expression
//{
//    DECLARE_TYPE_ID();
//    
//    element_expression_unbound_arg(size_t idx)
//        : element_expression(type_id)
//        , m_index(idx)
//    {
//    }
//
//    size_t index() const { return m_index; }
//
//protected:
//    size_t m_index;
//};