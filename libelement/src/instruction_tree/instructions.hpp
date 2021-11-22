#pragma once

#include "../ast/ast_internal.hpp"
#include "../ast/fwd.hpp"
#include "instruction_tree/fwd.hpp"
#include "typeutil.hpp"
#include "object_model/constraints/type.hpp"
#include "object_model/object_internal.hpp"

//STD
#include <string>
#include <vector>
#include <utility>
#include <numeric>
#include <unordered_map>
#include <set>

namespace element
{
//Element treats negative numbers and 0 as false
//todo: update to handle NAN's, once we've decided if they're truthy or falsy
[[nodiscard]] constexpr static bool to_bool(element_value value)
{
    return value > element_value{ 0 };
}

struct instruction : object, rtti_type<instruction>, std::enable_shared_from_this<instruction>
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
    [[nodiscard]] virtual bool get_constant_value(element_value& result) const = 0;

    [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const final
    {
        if (actual_type)
            return actual_type->matches_constraint(context, constraint);

        if (!constraint || constraint == constraint::any.get())
            return true;

        return false;
    }

    [[nodiscard]] const constraint* get_constraint() const final { return actual_type; }
    [[nodiscard]] std::string get_name() const override { return actual_type->get_name(); }
    [[nodiscard]] std::string typeof_info() const override;
    [[nodiscard]] std::shared_ptr<const instruction> to_instruction(const element_interpreter_ctx&) const final { return shared_from_this(); }
    
    type_const_ptr actual_type;

protected:
    explicit instruction(element_type_id t, type_const_ptr actual_type)
        : rtti_type(t)
        , actual_type(std::move(actual_type))
    {
    }

    std::vector<instruction_const_shared_ptr> m_dependents;
    element_type_id m_type_id = 0;
    int m_size = 0;

    [[nodiscard]] object_const_shared_ptr compile(
        const compilation_context& context,
        const source_information& source_info) const override;

public:
    [[nodiscard]] object_const_shared_ptr index(
        const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const override;
};

struct instruction_constant final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_constant(element_value val, type_const_ptr type = type::num.get());

    [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
        std::vector<object_const_shared_ptr> compiled_args,
        const source_information& source_info) const override;
    [[nodiscard]] element_value value() const { return m_value; }
    [[nodiscard]] size_t get_size() const override { return 1; }

    [[nodiscard]] bool get_constant_value(element_value& result) const override
    {
        result = value();
        return true;
    }

    [[nodiscard]] std::string to_string() const override
    {
        if (actual_type == type::boolean.get())
            return to_bool(m_value) ? "Bool = true" : "Bool = false";

        return fmt::format("Num = {:g}", m_value);
    }

    bool operator<(const instruction_constant& other) const noexcept
    {
        if (actual_type != other.actual_type)
            return actual_type < other.actual_type;

        const bool ma_nan = std::isnan(m_value);
        const bool yer_nan = std::isnan(other.m_value);

        if (ma_nan != yer_nan)
            return ma_nan < yer_nan;
        
        return m_value < other.m_value;
    }

private:
    element_value m_value;
};

//User-provided input from the boundary
struct instruction_input final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_input(size_t scope, size_t input_index, type_const_ptr type)
        : instruction(type_id, type)
        , m_scope(scope)
        , m_index(input_index)
    {
    }

    [[nodiscard]] size_t scope() const { return m_scope; }
    [[nodiscard]] size_t index() const { return m_index; }
    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] bool is_constant() const override { return false; }
    [[nodiscard]] bool get_constant_value(element_value& result) const override { return false; }
    [[nodiscard]] std::string to_string() const override { return fmt::format("<scope {}, index {}>", m_scope, m_index); }

    bool operator<(const instruction_input& other) const noexcept
    {
        if (m_scope != other.m_scope)
            return m_scope < other.m_scope;
        
        if (m_index != other.m_index)
            return m_index < other.m_index;

        return actual_type < other.actual_type;
    }

private:
    size_t m_scope;
    size_t m_index;
};

struct instruction_serialised_structure final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_serialised_structure(std::vector<instruction_const_shared_ptr>&& deps, std::vector<std::string>&& deps_names, std::string&& type_name)
        : instruction(type_id, nullptr)
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

    [[nodiscard]] bool get_constant_value(element_value& result) const override
    {
        return (is_constant() && get_size() == 1) ? m_dependents[0]->get_constant_value(result) : false;
    }

    [[nodiscard]] std::string get_type_name() const
    {
        return m_debug_type_name;
    }

    [[nodiscard]] std::vector<std::string> get_field_names() const
    {
        return m_debug_dependents_names;
    }
    
    bool operator<(const instruction_serialised_structure& other) const noexcept
    {
        if (m_dependents != other.m_dependents)
            return m_dependents < other.m_dependents;
        
        //the debug information doesn't matter for correctness, but it does for debugging
        //so two different struct types with the same values are different trees
        //even though they could be represented as one tree

        if (m_debug_type_name != other.m_debug_type_name)
            return m_debug_type_name < other.m_debug_type_name;

        return m_debug_dependents_names < other.m_debug_dependents_names;
    }

private:
    std::vector<std::string> m_debug_dependents_names;
    std::string m_debug_type_name;
};

struct instruction_nullary final : public instruction
{
    DECLARE_TYPE_ID();

    using op = element_nullary_op;

    explicit instruction_nullary(op t, type_const_ptr actual_type)
        : instruction(type_id, std::move(actual_type))
        , m_op(t)
    {

    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] size_t get_size() const override { return 1; }

    [[nodiscard]] bool is_constant() const override { return true; }
    [[nodiscard]] bool get_constant_value(element_value& result) const override;
    
    [[nodiscard]] bool operator<(const instruction_nullary& other) const noexcept
    {
        if (m_op != other.m_op)
            return m_op < other.m_op;

        return actual_type < other.actual_type;
    }

private:
    op m_op;
};

struct instruction_unary final : public instruction
{
    DECLARE_TYPE_ID();

    using op = element_unary_op;

    explicit instruction_unary(op t, instruction_const_shared_ptr input, type_const_ptr actual_type)
        : instruction(type_id, std::move(actual_type))
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(input));
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] const instruction_const_shared_ptr& input() const { return m_dependents[0]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] bool get_constant_value(element_value& result) const override;

    bool operator<(const instruction_unary& other) const noexcept
    {
        if (m_op != other.m_op) 
            return m_op < other.m_op;
        
        if (input() != other.input())
            return input() < other.input();

        return actual_type < other.actual_type;
    }

private:
    op m_op;
};

struct instruction_binary final : public instruction
{
    DECLARE_TYPE_ID();

    using op = element_binary_op;

    explicit instruction_binary(op t, instruction_const_shared_ptr in1, instruction_const_shared_ptr in2, type_const_ptr actual_type)
        : instruction(type_id, std::move(actual_type))
        , m_op(t)
    {
        m_dependents.emplace_back(std::move(in1));
        m_dependents.emplace_back(std::move(in2));
    }

    [[nodiscard]] op operation() const { return m_op; }
    [[nodiscard]] const instruction_const_shared_ptr& input1() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& input2() const { return m_dependents[1]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] bool get_constant_value(element_value& result) const override;

    bool operator<(const instruction_binary& other) const noexcept
    {
        if (operation() != other.operation())
            return operation() < other.operation();

        if (input1() != other.input1())
            return input1() < other.input1();

        if (input2() != other.input2())
            return input2() < other.input2();

        return actual_type < other.actual_type;
    }

private:
    op m_op;
};

struct instruction_if final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_if(instruction_const_shared_ptr predicate, instruction_const_shared_ptr if_true, instruction_const_shared_ptr if_false);
    [[nodiscard]] const instruction_const_shared_ptr& predicate() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& if_true() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& if_false() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] bool get_constant_value(element_value& result) const override;

    [[nodiscard]] bool operator<(const instruction_if& other) const noexcept
    {
        if (predicate() != other.predicate())
            return predicate() < other.predicate();

        if (if_true() != other.if_true())
            return if_true() < other.if_true();

        return if_false() < other.if_false();
    }
};

struct instruction_for final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_for(instruction_const_shared_ptr initial, instruction_const_shared_ptr condition, instruction_const_shared_ptr body, std::set<std::shared_ptr<const instruction_input>> inputs);
    [[nodiscard]] const instruction_const_shared_ptr& initial() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& condition() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& body() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return m_dependents[0]->get_size(); }
    [[nodiscard]] bool get_constant_value(element_value& result) const override { return false; }

    [[nodiscard]] bool is_input(const instruction_input& input) const
    {
        const auto it = std::find_if(inputs.begin(), inputs.end(), [&input](const auto& our_input) {
            return &input == our_input.get();
        });

        return it != inputs.end();
    }

    bool operator<(const instruction_for& other) const noexcept
    {
        if (initial() != other.initial())
            return initial() < other.initial();

        if (condition() != other.condition())
            return condition() < other.condition();

        if (body() != other.body())
            return body() < other.body();

        //in theory these shouldn't be different if the others are the same, but just in case
        return inputs < other.inputs;
    }

    std::set<std::shared_ptr<const instruction_input>> inputs;
};

struct instruction_fold final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_fold(instruction_const_shared_ptr list, instruction_const_shared_ptr initial, instruction_const_shared_ptr accumulator);
    [[nodiscard]] const instruction_const_shared_ptr& list() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& initial() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& accumulator() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return m_dependents[1]->get_size(); }
    [[nodiscard]] bool get_constant_value(element_value& result) const override { return false; }
};

struct instruction_indexer final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_indexer(std::shared_ptr<const instruction_for> for_instruction, int index, type_const_ptr type);

    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] bool get_constant_value(element_value& result) const override { return false; }

    [[nodiscard]] const instruction_const_shared_ptr& for_instruction() const { return m_dependents[0]; }

    bool operator<(const instruction_indexer& other) const noexcept
    {
        if (for_instruction() != other.for_instruction())
            return for_instruction() < other.for_instruction();

        if (index != other.index)
            return index < other.index;

        return actual_type < other.actual_type;
    }

    int index;
};

struct instruction_select final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_select(instruction_const_shared_ptr selector, std::vector<instruction_const_shared_ptr> options);

    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] const instruction_const_shared_ptr& options_at(size_t idx) const { return m_dependents[idx + 1]; }
    [[nodiscard]] size_t options_count() const { return m_dependents.size() - 1; };
    [[nodiscard]] const instruction_const_shared_ptr& selector() const { return m_dependents[0]; };
    [[nodiscard]] bool get_constant_value(element_value& result) const override;

    [[nodiscard]] bool operator<(const instruction_select& other) const noexcept
    {
        if (selector() != other.selector())
            return selector() < other.selector();

        return m_dependents < other.m_dependents;
    }
};

instruction_const_shared_ptr optimise_binary(element_interpreter_ctx* interpreter, const instruction_binary& binary);

} // namespace element