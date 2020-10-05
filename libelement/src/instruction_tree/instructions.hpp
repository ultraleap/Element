#pragma once

#include <string>
#include <vector>
#include <utility>
#include <numeric>
#include <unordered_map>

#include "../ast/ast_internal.hpp"
#include "../ast/fwd.hpp"
#include "instruction_tree/fwd.hpp"
#include "typeutil.hpp"
#include "object_model/constraints/type.hpp"
#include "object_model/object.hpp"

struct element_instruction : element::object, rtti_type<element_instruction>, std::enable_shared_from_this<element_instruction>
{
public:
    [[nodiscard]] const std::vector<instruction_const_shared_ptr>& dependents() const { return m_dependents; }
    std::vector<instruction_const_shared_ptr>& dependents() { return m_dependents; }

    [[nodiscard]] virtual size_t get_size() const { return m_size; }
    [[nodiscard]] bool is_constant() const override
    {
        auto is_constant = true;
        for (const auto& item : m_dependents)
            is_constant &= item->is_constant();

        return is_constant;
    }

    [[nodiscard]] bool matches_constraint(const element::compilation_context& context, const element::constraint* constraint) const final
    {
        if (actual_type)
            return actual_type->matches_constraint(context, constraint);

        if (!constraint || constraint == element::constraint::any.get())
            return true;

        return false;
    }

    [[nodiscard]] const element::constraint* get_constraint() const final { return actual_type; };

    [[nodiscard]] std::string typeof_info() const override;

    [[nodiscard]] std::shared_ptr<const element_instruction> to_instruction() const final { return shared_from_this(); };

    //todo: this is changed by the num constructor, but it should actually copy the instruction with the new type
    mutable element::type_const_ptr actual_type;

protected:
    explicit element_instruction(element_type_id t, element::type_const_ptr actual_type)
        : rtti_type(t)
        , actual_type(std::move(actual_type))
    {
    }

    std::vector<instruction_const_shared_ptr> m_dependents;
    element_type_id m_type_id = 0;
    int m_size = 0;

    [[nodiscard]] element::object_const_shared_ptr compile(
        const element::compilation_context& context,
        const element::source_information& source_info) const override;

public:
    [[nodiscard]] element::object_const_shared_ptr index(
        const element::compilation_context& context,
        const element::identifier& name,
        const element::source_information& source_info) const override;
};

struct element_instruction_constant final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_constant(element_value val);

    [[nodiscard]] virtual element::object_const_shared_ptr call(const element::compilation_context& context,
                                                                std::vector<element::object_const_shared_ptr> compiled_args,
                                                                const element::source_information& source_info) const;
    [[nodiscard]] element_value value() const { return m_value; }
    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] std::string to_string() const override
    {
        if (actual_type == element::type::boolean.get())
            return m_value > 0 ? "true" : "false";

        return fmt::format("{:g}", m_value);
    }

private:
    element_value m_value;
};

//User-provided input from the boundary
struct element_instruction_input final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_input(size_t scope, size_t input_index)
        : element_instruction(type_id, nullptr)
        , m_scope(scope)
        , m_index(input_index)
    {
    }

    [[nodiscard]] size_t scope() const { return m_scope; }
    [[nodiscard]] size_t index() const { return m_index; }
    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] bool is_constant() const override { return false; }
    [[nodiscard]] std::string to_string() const override { return fmt::format("<scope {}, index {}>", m_scope, m_index); }

private:
    size_t m_scope;
    size_t m_index;
};

struct element_instruction_serialised_structure final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_serialised_structure(std::vector<instruction_const_shared_ptr>&& deps, std::vector<std::string>&& deps_names, std::string&& type_name)
        : element_instruction(type_id, nullptr)
        , m_debug_dependents_names(std::move(deps_names))
        , m_debug_type_name(std::move(type_name))
    {
        m_dependents = std::move(deps);
    }

    [[nodiscard]] instruction_const_shared_ptr output(size_t index) const
    {
        return m_dependents[index];
    }

    [[nodiscard]] size_t get_size() const override
    {
        return std::accumulate(m_dependents.begin(), m_dependents.end(), size_t(0),
                               [](size_t c, const auto& d) { return c + d->get_size(); });
    }

private:
    std::vector<std::string> m_debug_dependents_names;
    std::string m_debug_type_name;
};

struct element_instruction_nullary final : public element_instruction
{
    DECLARE_TYPE_ID();

    using op = element_nullary_op;

    explicit element_instruction_nullary(op t, element::type_const_ptr actual_type)
        : element_instruction(type_id, std::move(actual_type))
        , m_op(t)
    {
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] size_t get_size() const override { return 1; }

private:
    op m_op;
};

struct element_instruction_unary final : public element_instruction
{
    DECLARE_TYPE_ID();

    using op = element_unary_op;

    explicit element_instruction_unary(op t, instruction_const_shared_ptr input, element::type_const_ptr actual_type)
        : element_instruction(type_id, std::move(actual_type))
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(input));
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] const instruction_const_shared_ptr& input() const { return m_dependents[0]; }

    [[nodiscard]] size_t get_size() const override { return 1; }

private:
    op m_op;
};

struct element_instruction_binary final : public element_instruction
{
    DECLARE_TYPE_ID();

    using op = element_binary_op;

    explicit element_instruction_binary(op t, instruction_const_shared_ptr in1, instruction_const_shared_ptr in2, element::type_const_ptr actual_type)
        : element_instruction(type_id, std::move(actual_type))
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(in1));
        m_dependents.emplace_back(std::move(in2));
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] const instruction_const_shared_ptr& input1() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& input2() const { return m_dependents[1]; }

    [[nodiscard]] size_t get_size() const override { return 1; }

private:
    op m_op;
};

struct element_instruction_if final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_if(instruction_const_shared_ptr predicate, instruction_const_shared_ptr if_true, instruction_const_shared_ptr if_false);
    [[nodiscard]] const instruction_const_shared_ptr& predicate() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& if_true() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& if_false() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
};

struct element_instruction_for final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_for(instruction_const_shared_ptr initial, instruction_const_shared_ptr condition, instruction_const_shared_ptr body);
    [[nodiscard]] const instruction_const_shared_ptr& initial() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& condition() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& body() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return m_dependents[0]->get_size(); }
};

struct element_instruction_fold final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_fold(instruction_const_shared_ptr list, instruction_const_shared_ptr initial, instruction_const_shared_ptr accumulator);
    [[nodiscard]] const instruction_const_shared_ptr& list() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& initial() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& accumulator() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return m_dependents[1]->get_size(); }
};

struct element_instruction_indexer final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_indexer(std::shared_ptr<const element_instruction_for> for_instruction, int index, element::type_const_ptr type);

    [[nodiscard]] size_t get_size() const override { return 1; }

    std::shared_ptr<const element_instruction_for> for_instruction;
    int index;
};

struct element_instruction_select final : public element_instruction
{
    DECLARE_TYPE_ID();

    explicit element_instruction_select(instruction_const_shared_ptr selector, std::vector<instruction_const_shared_ptr> options);

    [[nodiscard]] size_t get_size() const override { return 1; }

    instruction_const_shared_ptr selector;
    std::vector<instruction_const_shared_ptr> options;
};

////
//// instruction groups
////
//struct element_instruction_group : public element_instruction
//{
//    DECLARE_TYPE_ID();
//
//    //todo: do we still need instruction groups?
//protected:
//    element_instruction_group()
//        : element_instruction(type_id)
//    {
//    }
//
//    // virtual size_t group_size() const = 0;
//};
//
//struct element_instruction_unbound_arg : public element_instruction
//{
//    DECLARE_TYPE_ID();
//
//    element_instruction_unbound_arg(size_t idx)
//        : element_instruction(type_id)
//        , m_index(idx)
//    {
//    }
//
//    size_t index() const { return m_index; }
//
//protected:
//    size_t m_index;
//};