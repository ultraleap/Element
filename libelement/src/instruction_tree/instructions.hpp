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
    [[nodiscard]] std::shared_ptr<const instruction> to_instruction() const final { return shared_from_this(); }

    //todo: this is changed by the num constructor, but it should actually copy the instruction with the new type
    mutable type_const_ptr actual_type;

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

    explicit instruction_constant(element_value val);

    [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
        std::vector<object_const_shared_ptr> compiled_args,
        const source_information& source_info) const override;
    [[nodiscard]] element_value value() const { return m_value; }
    [[nodiscard]] size_t get_size() const override { return 1; }
    [[nodiscard]] std::string to_string() const override
    {
        if (actual_type == type::boolean.get())
            return to_bool(m_value) ? "Bool = true" : "Bool = false";

        return fmt::format("Num = {:g}", m_value);
    }

private:
    element_value m_value;
};

//User-provided input from the boundary
struct instruction_input final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_input(size_t scope, size_t input_index)
        : instruction(type_id, nullptr)
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

    [[nodiscard]] std::string get_type_name() const
    {
        return m_debug_type_name;
    }

    [[nodiscard]] std::vector<std::string> get_field_names() const
    {
        return m_debug_dependents_names;
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
};

struct instruction_for final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_for(instruction_const_shared_ptr initial, instruction_const_shared_ptr condition, instruction_const_shared_ptr body, std::set<std::shared_ptr<const instruction_input>> inputs);
    [[nodiscard]] const instruction_const_shared_ptr& initial() const { return m_dependents[0]; }
    [[nodiscard]] const instruction_const_shared_ptr& condition() const { return m_dependents[1]; }
    [[nodiscard]] const instruction_const_shared_ptr& body() const { return m_dependents[2]; }

    [[nodiscard]] size_t get_size() const override { return m_dependents[0]->get_size(); }

    [[nodiscard]] bool is_input(const instruction_input& input) const
    {
        const auto it = std::find_if(inputs.begin(), inputs.end(), [&input](const auto& our_input) {
            return &input == our_input.get();
        });

        return it != inputs.end();
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
};

struct instruction_indexer final : public instruction
{
    DECLARE_TYPE_ID();

    explicit instruction_indexer(std::shared_ptr<const instruction_for> for_instruction, int index, type_const_ptr type);

    [[nodiscard]] size_t get_size() const override { return 1; }

    [[nodiscard]] const instruction_const_shared_ptr& for_instruction() const { return m_dependents[0]; }
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
};

//do some additional peephole optimisations based on known operations and operands
static instruction_const_shared_ptr optimise_binary(const instruction_binary& binary)
{
    const auto* input1_as_const = binary.input1()->as<const instruction_constant>();
    const auto* input2_as_const = binary.input2()->as<const instruction_constant>();

    //if it's a numerical op and one of the operands is NaN, then the result is NaN
    //todo: can we also optimise for +/- Inf?
    if (binary.operation() < element_binary_op::and_) {
        if (input1_as_const && std::isnan(input1_as_const->value()))
            return binary.input1();

        if (input2_as_const && std::isnan(input2_as_const->value()))
            return binary.input2();
    }

    switch (binary.operation()) {
    case element_binary_op::add: {
        if (input1_as_const && input1_as_const->value() == 0.0f)
            return binary.input2();

        if (input2_as_const && input2_as_const->value() == 0.0f)
            return binary.input1();

        //todo: could transform identical adds to mul(input, 2) if that's faster?
        //probably machine architecture dependent, should be an optimisation done by the target (e.g. LMNT)

        break;
    }

    case element_binary_op::sub: {
        if (input2_as_const && input2_as_const->value() == 0.0f)
            return binary.input1();

        break;
    }

    case element_binary_op::mul: {
        if (input1_as_const && input1_as_const->value() == 1.0f)
            return binary.input2();

        if (input2_as_const && input2_as_const->value() == 1.0f)
            return binary.input1();

        // NaN or Inf * 0 = NaN, and since that is valid user input, we can't do that optimisation

        break;
    }

    case element_binary_op::div: {
        if (input2_as_const && input2_as_const->value() == 1.0f)
            return binary.input1();

        // We can't optimise for division by 0

        // todo: could transform divs to muls if that's faster

        break;
    }

    default: {
        //todo: optimise other operators
        break;
    }
    }

    return nullptr;
}
} // namespace element