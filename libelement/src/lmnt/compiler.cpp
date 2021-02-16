#include "lmnt/compiler.hpp"

#include <algorithm>
#include <vector>
#include <unordered_map>

#define U16_LO(x) static_cast<uint16_t>((x) & 0xFFFF)
#define U16_HI(x) static_cast<uint16_t>(((x) >> 16) & 0xFFFF)

struct virtual_result
{
    const element::instruction* instruction = nullptr;
    uint16_t count = 0;
    // filled in during compilation process
    uint16_t stack_index = 0;
    size_t inst_index_set = 0;
    size_t inst_index_last_used = 0;
};

struct compiler_state
{
    const element_lmnt_compiler_ctx& ctx;
    std::vector<element_value> constants;
    uint16_t inputs_count;

    std::unordered_map<const element::instruction*, size_t> results;
    std::vector<virtual_result> virtual_results;

    element_result add_constant(element_value value)
    {
        if (constants.size() >= UINT16_MAX)
            return ELEMENT_ERROR_UNKNOWN;
        if (std::find(constants.begin(), constants.end(), value) == constants.end())
            constants.emplace_back(value);
        return ELEMENT_OK;
    }

    element_result find_constant(element_value value, uint16_t& index)
    {
        if (auto it = std::find(constants.begin(), constants.end(), value); it != constants.end())
        {
            index = static_cast<uint16_t>(std::distance(constants.begin(), it));
            return ELEMENT_OK;
        }
        else
        {
            return ELEMENT_ERROR_NOT_FOUND;
        }
    }

    element_result find_virtual_result(const element::instruction* in, virtual_result& vr)
    {
        if (auto it = results.find(in); it != results.end() && it->second < virtual_results.size())
        {
            vr = virtual_results[it->second];
            return ELEMENT_OK;
        }
        else
        {
            return ELEMENT_ERROR_NOT_FOUND;
        }
    }
};


element_result create_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    virtual_result& result);

element_result compile_instruction(
    compiler_state& state,
    const element::instruction* expr,
    std::vector<lmnt_instruction>& output);


element_result copy_stack_values(const uint16_t src_index, const uint16_t dst_index, const uint16_t count, std::vector<lmnt_instruction>& output)
{
    if (src_index == dst_index)
        return ELEMENT_OK;

    const uint16_t countm4 = (count/4)*4;
    for (uint16_t i = 0; i < countm4; i += 4)
        output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNVV, uint16_t(src_index + i), 0, uint16_t(dst_index + i)});
    for (uint16_t i = countm4; i < count; ++i)
        output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNSS, uint16_t(src_index + i), 0, uint16_t(dst_index + i)});
    return ELEMENT_OK;
}


//
// Constant
//

static element_result create_virtual_constant(
    compiler_state& state,
    const element::instruction_constant& ec,
    virtual_result& result)
{
    result.instruction = &ec;
    result.count = 1;

    // TODO: constant candidates here?

    return ELEMENT_OK;
}

static element_result compile_constant_value(
    compiler_state& state,
    element_value value,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    uint16_t constant_idx;
    if (state.find_constant(value, constant_idx) == ELEMENT_OK) {
        // this constant is present in the "hard" constants list, so we don't need to do anything
        // output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNSS, constant_idx, 0, output_idx});
    } else {
        // encode the constant as part of the instruction
        uint32_t arg;
        assert(sizeof(arg) == sizeof(value));
        std::memcpy(&arg, &value, sizeof(value));
        const uint16_t arglo = static_cast<uint16_t>(arg & 0xFFFF);
        const uint16_t arghi = static_cast<uint16_t>((arg >> 16) & 0xFFFF);
        output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIBS, arglo, arghi, output_idx});
    }
    return ELEMENT_OK;
}

static element_result compile_constant(
    compiler_state& state,
    const element::instruction_constant& ec,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    return compile_constant_value(state, ec.value(), output_idx, output);
}


//
// Input
//

static element_result create_virtual_input(
    compiler_state& state,
    const element::instruction_input& ei,
    virtual_result& result)
{
    result.instruction = &ei;
    result.count = 1;
    return ELEMENT_OK;
}

static element_result compile_input(
    compiler_state& state,
    const element::instruction_input& ei,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    // straight copy from input[n] to output[m]
    copy_stack_values(static_cast<uint16_t>(state.constants.size() + ei.index()), output_idx, 1, output);
    return ELEMENT_OK;
}


//
// Serialised structure
//

static element_result create_virtual_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es,
    virtual_result& result)
{
    result.instruction = &es;
    result.count = 0;
    for (const auto& d : es.dependents())
    {
        virtual_result vr;
        ELEMENT_OK_OR_RETURN(create_virtual_result(state, d.get(), vr));
        result.count += vr.count;
    }
    return ELEMENT_OK;
}

static element_result compile_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    for (const auto& d : es.dependents())
    {
        ELEMENT_OK_OR_RETURN(compile_instruction(state, d.get(), output));
    }
    return ELEMENT_OK;
}


//
// Nullary
//

static element_result create_virtual_nullary(
    compiler_state& state,
    const element::instruction_nullary& en,
    virtual_result& result)
{
    result.instruction = &en;
    result.count = 1;
    return ELEMENT_OK;
}

static element_result compile_nullary(
    compiler_state& state,
    const element::instruction_nullary& en,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    element_value value;
    switch (en.operation())
    {
    //num
    case element::instruction_nullary::op::positive_infinity:
        value = std::numeric_limits<float>::infinity(); break;
    case element::instruction_nullary::op::negative_infinity:
        value = -std::numeric_limits<float>::infinity(); break;
    case element::instruction_nullary::op::nan:
        value = std::numeric_limits<float>::quiet_NaN(); break;

    //boolean
    case element::instruction_nullary::op::true_value:
        value = 1; break;
    case element::instruction_nullary::op::false_value:
        value = 0; break;
    default:
        assert(false);
        return ELEMENT_ERROR_UNKNOWN;
    }
    return compile_constant_value(state, value, output_idx, output);
}


//
// Unary
//

static element_result create_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu,
    virtual_result& result)
{
    result.instruction = &eu;
    result.count = 1;

    if (eu.dependents().size() != 1)
        return ELEMENT_ERROR_UNKNOWN;

    virtual_result arg_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eu.dependents()[0].get(), arg_vr));

    if (arg_vr.count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    // if we're doing boolean not we need to bring a couple of constants along
    if (eu.operation() == element::instruction_unary::op::not_)
    {
        ELEMENT_OK_OR_RETURN(state.add_constant(-1));
        ELEMENT_OK_OR_RETURN(state.add_constant(1));
    }

    return ELEMENT_OK;
}

static element_result compile_unary(
    compiler_state& state,
    const element::instruction_unary& eu,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    if (eu.dependents().size() != 1)
        return ELEMENT_ERROR_UNKNOWN;
    // get the argument and ensure it's only 1 wide
    virtual_result arg_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(eu.dependents()[0].get(), arg_vr));
    if (arg_vr.count != 1)
        return ELEMENT_ERROR_UNKNOWN;
    // compile the argument
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg_vr.instruction, output));

    if (eu.operation() == element::instruction_unary::op::not_)
    {
        // boolean value is defined as >0 being true and <=0 as being false
        // this requires a bit of creative logic to ensure correctness for all inputs
        // ceil --> min(1) --> mul(-1) --> add(1)
        // for x < 0:  ceil --> (x <= 0)  --> min(1) --> no change --> mul(-1) --> (x > 0)   --> add(1) --> (x > 0)
        // for x == 0: ceil --> no change --> min(1) --> no change --> mul(-1) --> no change --> add(1) --> (x == 1)
        // for x <= 1: ceil --> (x == 1)  --> min(1) --> no change --> mul(-1) --> (x == -1) --> add(1) --> (x == 0)
        // for x > 1:  ceil --> (x > 1)   --> min(1) --> (x == 1)  --> mul(-1) --> (x == -1) --> add(1) --> (x == 0)
        uint16_t const_minus1, const_plus1;
        ELEMENT_OK_OR_RETURN(state.find_constant(-1, const_minus1));
        ELEMENT_OK_OR_RETURN(state.find_constant(1, const_plus1));
        output.emplace_back(lmnt_instruction{LMNT_OP_CEILS, arg_vr.stack_index, 0, output_idx});
        output.emplace_back(lmnt_instruction{LMNT_OP_MINSS, output_idx, const_plus1, output_idx});
        output.emplace_back(lmnt_instruction{LMNT_OP_MULSS, output_idx, const_minus1, output_idx});
        output.emplace_back(lmnt_instruction{LMNT_OP_ADDSS, output_idx, const_plus1, output_idx});
    }
    else
    {
        lmnt_opcode op;
        switch (eu.operation())
        {
        case element::instruction_unary::op::abs:   op = LMNT_OP_ABSS;   break;
        case element::instruction_unary::op::acos:  op = LMNT_OP_ACOS;   break;
        case element::instruction_unary::op::asin:  op = LMNT_OP_ASIN;   break;
        case element::instruction_unary::op::atan:  op = LMNT_OP_ATAN;   break;
        case element::instruction_unary::op::ceil:  op = LMNT_OP_CEILS;  break;
        case element::instruction_unary::op::cos:   op = LMNT_OP_COS;    break;
        case element::instruction_unary::op::floor: op = LMNT_OP_FLOORS; break;
        case element::instruction_unary::op::ln:    op = LMNT_OP_LN;     break;
        case element::instruction_unary::op::sin:   op = LMNT_OP_SIN;    break;
        case element::instruction_unary::op::tan:   op = LMNT_OP_TAN;    break;
        default: assert(false); return ELEMENT_ERROR_UNKNOWN;
        }
        output.emplace_back(lmnt_instruction{op, arg_vr.stack_index, 0, output_idx});
    }
    return ELEMENT_OK;
}


//
// Binary
//

static element_result create_virtual_binary(
    compiler_state& state,
    const element::instruction_binary& eb,
    virtual_result& result)
{
    result.instruction = &eb;
    result.count = 1;

    return ELEMENT_OK;
}

static element_result compile_binary(
    compiler_state& state,
    const element::instruction_binary& eb,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    if (eb.dependents().size() != 2)
        return ELEMENT_ERROR_UNKNOWN;
    // get the arguments and ensure they're only 1 wide
    virtual_result arg1_vr, arg2_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(eb.dependents()[0].get(), arg1_vr));
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(eb.dependents()[1].get(), arg2_vr));
    if (arg1_vr.count != 1 || arg2_vr.count != 1)
        return ELEMENT_ERROR_UNKNOWN;
    // compile the arguments
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg1_vr.instruction, output));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg2_vr.instruction, output));

    lmnt_opcode op;
    switch (eb.operation())
    {
        //num
        case element::instruction_binary::op::add:   op = LMNT_OP_ADDSS; goto num_operation;
        case element::instruction_binary::op::atan2: op = LMNT_OP_ATAN2; goto num_operation;
        case element::instruction_binary::op::div:   op = LMNT_OP_DIVSS; goto num_operation;
        case element::instruction_binary::op::max:   op = LMNT_OP_MAXSS; goto num_operation;
        case element::instruction_binary::op::min:   op = LMNT_OP_MINSS; goto num_operation;
        case element::instruction_binary::op::mul:   op = LMNT_OP_MULSS; goto num_operation;
        case element::instruction_binary::op::pow:   op = LMNT_OP_POWSS; goto num_operation;
        case element::instruction_binary::op::rem:   op = LMNT_OP_MODSS; goto num_operation;
        case element::instruction_binary::op::sub:   op = LMNT_OP_SUBSS; goto num_operation;
    num_operation:
            output.emplace_back(lmnt_instruction{op, arg1_vr.stack_index, arg2_vr.stack_index, output_idx});
            return ELEMENT_OK;

        case element::instruction_binary::op::log:
            // TODO: check if we've got a constant base we can work with
            output.emplace_back(lmnt_instruction{LMNT_OP_LOG, arg1_vr.stack_index, arg2_vr.stack_index, output_idx});
            return ELEMENT_OK;

        default:
            break;
    }

    const uint32_t start_idx = static_cast<uint32_t>(output.size()); // index of the next op

    switch(eb.operation())
    {
        //boolean
        case element::instruction_binary::op::and_:
            // (arg1 > 0.0 && arg2 > 0.0)
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg1_vr.stack_index, 0, 0});                            // + 0
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 1
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg2_vr.stack_index, 0, 0});                            // + 2
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 3
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 1, 0, output_idx});                                // + 4
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH,    0, U16_LO(start_idx + 7), U16_HI(start_idx + 7)}); // + 5
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 0, 0, output_idx});                                // + 6
            return ELEMENT_OK;
        case element::instruction_binary::op::or_:
            // (arg1 > 0.0 || arg2 > 0.0)
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg1_vr.stack_index, 0, 0});                            // + 0
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCGT, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 1
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg2_vr.stack_index, 0, 0});                            // + 2
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCGT, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 3
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 0, 0, output_idx});                                // + 4
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, U16_LO(start_idx + 7), U16_HI(start_idx + 7)});    // + 5
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 1, 0, output_idx});                                // + 6
            return ELEMENT_OK;

        //comparison
        case element::instruction_binary::op::eq:  op = LMNT_OP_BRANCHCEQ; goto cmp_operation;
        case element::instruction_binary::op::neq: op = LMNT_OP_BRANCHCNE; goto cmp_operation;
        case element::instruction_binary::op::lt:  op = LMNT_OP_BRANCHCLT; goto cmp_operation;
        case element::instruction_binary::op::leq: op = LMNT_OP_BRANCHCLE; goto cmp_operation;
        case element::instruction_binary::op::gt:  op = LMNT_OP_BRANCHCGT; goto cmp_operation;
        case element::instruction_binary::op::geq: op = LMNT_OP_BRANCHCGE; goto cmp_operation;
    cmp_operation:
            output.emplace_back(lmnt_instruction{LMNT_OP_CMP, arg1_vr.stack_index, arg2_vr.stack_index, 0});        // + 0
            output.emplace_back(lmnt_instruction{op, 0, U16_LO(start_idx + 4), U16_HI(start_idx + 4)});             // + 1
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 0, 0, output_idx});                             // + 2
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, U16_LO(start_idx + 5), U16_HI(start_idx + 5)}); // + 3
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 1, 0, output_idx});                             // + 4
            return ELEMENT_OK;

        default:
            assert(false);
            return ELEMENT_ERROR_UNKNOWN;
    }
}


//
// If
//

static element_result create_virtual_if(
    compiler_state& state,
    const element::instruction_if& ei,
    virtual_result& result)
{
    if (ei.dependents().size() != 3)
        return ELEMENT_ERROR_UNKNOWN;

    // ensure the predicate is only 1 wide
    virtual_result predicate_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.dependents()[0].get(), predicate_vr));
    if (predicate_vr.count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    // ensure both results of the if are at least the same size
    virtual_result true_vr, false_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.dependents()[1].get(), true_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.dependents()[2].get(), false_vr));
    if (true_vr.count != false_vr.count)
        return ELEMENT_ERROR_UNKNOWN;

    result.instruction = &ei;
    result.count = true_vr.count;

    return ELEMENT_OK;
}

static element_result compile_if(
    compiler_state& state,
    const element::instruction_if& ei,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    virtual_result predicate_vr, true_vr, false_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ei.dependents()[0].get(), predicate_vr));
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ei.dependents()[1].get(), true_vr));
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ei.dependents()[2].get(), false_vr));

    // compile the predicate
    ELEMENT_OK_OR_RETURN(compile_instruction(state, predicate_vr.instruction, output));
    // make branch logic
    output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, predicate_vr.stack_index, 0, 0});
    const size_t predicate_branchcle_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, 0, 0}); // target filled in at the end
    // compile the true/false branches
    ELEMENT_OK_OR_RETURN(compile_instruction(state, true_vr.instruction, output));
    copy_stack_values(true_vr.stack_index, output_idx, true_vr.count, output);
    const size_t branch_past_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, 0, 0}); // target filled in at the end
    const size_t false_idx = output.size();
    ELEMENT_OK_OR_RETURN(compile_instruction(state, false_vr.instruction, output));
    copy_stack_values(false_vr.stack_index, output_idx, true_vr.count, output);
    const size_t past_idx = output.size();

    // fill in targets for our branches
    output[predicate_branchcle_idx].arg2 = U16_LO(false_idx);
    output[predicate_branchcle_idx].arg3 = U16_HI(false_idx);
    output[branch_past_idx].arg2 = U16_LO(past_idx);
    output[branch_past_idx].arg3 = U16_HI(past_idx);

    return ELEMENT_OK;
}


//
// For
//

static element_result create_virtual_for(
    compiler_state& state,
    const element::instruction_for& ef,
    virtual_result& result)
{
    if (ef.dependents().size() != 3)
        return ELEMENT_ERROR_UNKNOWN;

    virtual_result initial_vr, condition_vr, body_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.dependents()[0].get(), initial_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.dependents()[1].get(), condition_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.dependents()[2].get(), body_vr));
    if (condition_vr.count != 1)
        return ELEMENT_ERROR_UNKNOWN;
    if (initial_vr.count != body_vr.count)
        return ELEMENT_ERROR_UNKNOWN;

    result.instruction = &ef;
    result.count = initial_vr.count;

    return ELEMENT_OK;
}

static element_result compile_for(
    compiler_state& state,
    const element::instruction_for& ef,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    virtual_result initial_vr, condition_vr, body_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ef.dependents()[0].get(), initial_vr));
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ef.dependents()[1].get(), condition_vr));
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ef.dependents()[2].get(), body_vr));

    // compile the initial state
    ELEMENT_OK_OR_RETURN(compile_instruction(state, initial_vr.instruction, output));
    copy_stack_values(initial_vr.stack_index, output_idx, initial_vr.count, output);
    // compile condition logic
    const size_t condition_index = output.size();
    ELEMENT_OK_OR_RETURN(compile_instruction(state, condition_vr.instruction, output));
    output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, condition_vr.stack_index, 0, 0});
    const size_t condition_branchcle_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, 0, 0}); // target filled in at the end
    // compile the loop body
    ELEMENT_OK_OR_RETURN(compile_instruction(state, body_vr.instruction, output));
    copy_stack_values(body_vr.stack_index, output_idx, body_vr.count, output);
    const size_t branch_past_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, U16_LO(condition_index), U16_HI(condition_index)}); // branch --> condition
    const size_t past_idx = output.size();

    // fill in targets for the exit branch
    output[condition_branchcle_idx].arg2 = U16_LO(past_idx);
    output[condition_branchcle_idx].arg3 = U16_HI(past_idx);

    return ELEMENT_OK;
}


//
// Indexer
//

static element_result create_virtual_indexer(
    compiler_state& state,
    const element::instruction_indexer& ei,
    virtual_result& result)
{
    virtual_result for_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.for_instruction.get(), for_vr));
    if (ei.index >= for_vr.count)
        return ELEMENT_ERROR_UNKNOWN;

    result.instruction = &ei;
    result.count = 1;

    return ELEMENT_OK;
}

static element_result compile_indexer(
    compiler_state& state,
    const element::instruction_indexer& ei,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    virtual_result for_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(ei.for_instruction.get(), for_vr));

    const uint16_t for_entry_idx = static_cast<uint16_t>(for_vr.stack_index + ei.index);
    copy_stack_values(for_entry_idx, output_idx, 1, output);
    return ELEMENT_OK;
}


//
// Select
//

static element_result create_virtual_select(
    compiler_state& state,
    const element::instruction_select& es,
    virtual_result& result)
{
    virtual_result selector_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, es.selector.get(), selector_vr));
    if (selector_vr.count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    if (es.options.empty())
        return ELEMENT_ERROR_UNKNOWN;

    ELEMENT_OK_OR_RETURN(state.add_constant(0));
    ELEMENT_OK_OR_RETURN(state.add_constant(1));
    ELEMENT_OK_OR_RETURN(state.add_constant(element_value(es.options.size() - 1)));

    uint16_t max_count = 0;
    for (const auto& o : es.options)
    {
        virtual_result option_vr;
        ELEMENT_OK_OR_RETURN(create_virtual_result(state, o.get(), option_vr));
        max_count = (std::max)(max_count, static_cast<uint16_t>(option_vr.count));
    }

    result.instruction = &es;
    result.count = max_count;

    return ELEMENT_OK;
}

static element_result compile_select(
    compiler_state& state,
    const element::instruction_select& es,
    const uint16_t output_idx,
    std::vector<lmnt_instruction>& output)
{
    const size_t opts_size = es.options.size();
    uint16_t zero_idx, one_idx, last_valid_idx;
    ELEMENT_OK_OR_RETURN(state.find_constant(0, zero_idx));
    ELEMENT_OK_OR_RETURN(state.find_constant(1, one_idx));
    ELEMENT_OK_OR_RETURN(state.find_constant(element_value(opts_size - 1), last_valid_idx));

    virtual_result selector_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(es.selector.get(), selector_vr));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, es.selector.get(), output));
    // clamp in range and ensure it's an integer
    output.emplace_back(lmnt_instruction{LMNT_OP_MAXSS,  selector_vr.stack_index, zero_idx, selector_vr.stack_index});
    output.emplace_back(lmnt_instruction{LMNT_OP_MINSS,  selector_vr.stack_index, last_valid_idx, selector_vr.stack_index});
    output.emplace_back(lmnt_instruction{LMNT_OP_TRUNCS, selector_vr.stack_index, 0, selector_vr.stack_index});

    const size_t branches_start_idx = output.size();
    for (size_t i = 0; i < opts_size; ++i)
    {
        output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, selector_vr.stack_index, 0, 0});
        output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCEQ, 0, 0, 0}); // target filled in later
        output.emplace_back(lmnt_instruction{LMNT_OP_SUBSS, selector_vr.stack_index, one_idx, selector_vr.stack_index});
    }

    virtual_result option0_vr;
    ELEMENT_OK_OR_RETURN(state.find_virtual_result(es.options[0].get(), option0_vr));
    const bool stacked = (option0_vr.stack_index == output_idx);

    std::vector<size_t> option_branch_indexes(opts_size);

    for (size_t i = 0; i < opts_size; ++i)
    {
        virtual_result option_vr;
        ELEMENT_OK_OR_RETURN(state.find_virtual_result(es.options[i].get(), option_vr));

        const size_t option_idx = output.size();
        // fill in the target index for this branch option
        output[branches_start_idx + i*3 + 1].arg2 = U16_LO(option_idx);
        output[branches_start_idx + i*3 + 1].arg3 = U16_HI(option_idx);
        ELEMENT_OK_OR_RETURN(compile_instruction(state, es.options[i].get(), output));
        copy_stack_values(option_vr.stack_index, output_idx, option_vr.count, output);
        option_branch_indexes[i] = output.size();
        output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, 0, 0}); // target filled in later
    }

    const size_t past_idx = output.size();
    for (size_t i = 0; i < opts_size; ++i)
    {
        output[option_branch_indexes[i]].arg2 = U16_LO(past_idx);
        output[option_branch_indexes[i]].arg3 = U16_HI(past_idx);
    }

    return ELEMENT_OK;
}



static element_result create_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    virtual_result& result)
{
    // CSE
    if (auto it = state.results.find(expr); it != state.results.end())
    {
        result = state.virtual_results[it->second];
        return ELEMENT_OK;
    }

    element_result oresult = ELEMENT_ERROR_NO_IMPL;

    if (const auto* ec = expr->as<element::instruction_constant>())
        oresult = create_virtual_constant(state, *ec, result);

    if (const auto* ei = expr->as<element::instruction_input>())
        oresult = create_virtual_input(state, *ei, result);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        oresult = create_virtual_serialised_structure(state, *es, result);

    if (const auto* en = expr->as<element::instruction_nullary>())
        oresult = create_virtual_nullary(state, *en, result);

    if (const auto* eu = expr->as<element::instruction_unary>())
        oresult = create_virtual_unary(state, *eu, result);

    if (const auto* eb = expr->as<element::instruction_binary>())
        oresult = create_virtual_binary(state, *eb, result);

    if (const auto* ei = expr->as<element::instruction_if>())
        oresult = create_virtual_if(state, *ei, result);

    if (const auto* ef = expr->as<element::instruction_for>())
        oresult = create_virtual_for(state, *ef, result);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        oresult = create_virtual_indexer(state, *ei, result);

    if (const auto* sel = expr->as<element::instruction_select>())
        oresult = create_virtual_select(state, *sel, result);

    if (oresult == ELEMENT_OK)
    {
        state.results.emplace(expr, state.virtual_results.size());
        state.virtual_results.emplace_back(result);
    }

    return oresult;
}

static element_result compile_instruction(
    compiler_state& state,
    const element::instruction* expr,
    std::vector<lmnt_instruction>& output)
{
    auto results_it = state.results.find(expr);
    if (results_it == state.results.end())
        return ELEMENT_ERROR_UNKNOWN;

    const virtual_result& vr = state.virtual_results[results_it->second];

    if (const auto* ec = expr->as<element::instruction_constant>())
        return compile_constant(state, *ec, vr.stack_index, output);

    if (const auto* ei = expr->as<element::instruction_input>())
        return compile_input(state, *ei, vr.stack_index, output);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        return compile_serialised_structure(state, *es, vr.stack_index, output);

    if (const auto* en = expr->as<element::instruction_nullary>())
        return compile_nullary(state, *en, vr.stack_index, output);

    if (const auto* eu = expr->as<element::instruction_unary>())
        return compile_unary(state, *eu, vr.stack_index, output);

    if (const auto* eb = expr->as<element::instruction_binary>())
        return compile_binary(state, *eb, vr.stack_index, output);

    if (const auto* ei = expr->as<element::instruction_if>())
        return compile_if(state, *ei, vr.stack_index, output);

    if (const auto* ef = expr->as<element::instruction_for>())
        return compile_for(state, *ef, vr.stack_index, output);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        return compile_indexer(state, *ei, vr.stack_index, output);

    if (const auto* sel = expr->as<element::instruction_select>())
        return compile_select(state, *sel, vr.stack_index, output);

    return ELEMENT_ERROR_NO_IMPL;
}


element_result element_lmnt_find_constants(
    const element_lmnt_compiler_ctx& ctx,
    const element::instruction_const_shared_ptr& expr,
    std::unordered_map<element_value, size_t>& candidates)
{
    if (const auto* ec = expr->as<element::instruction_constant>())
    {
        const element_value value = ec->value();
        auto it = candidates.find(value);
        if (it != candidates.end())
            ++it->second;
        else
            candidates.emplace(value, 1);
    }
    for (const auto& dep : expr->dependents())
    {
        ELEMENT_OK_OR_RETURN(element_lmnt_find_constants(ctx, dep, candidates));
    }
    return ELEMENT_OK;
}

element_result element_lmnt_compile_function(
    const element_lmnt_compiler_ctx& ctx,
    const element::instruction_const_shared_ptr instruction,
    std::vector<element_value> constants,
    const size_t inputs_count,
    std::vector<lmnt_instruction>& output)
{
    compiler_state state { ctx, std::move(constants), static_cast<uint16_t>(inputs_count) };
    // TODO: check for single constant fast path
    virtual_result vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, instruction.get(), vr));
    return compile_instruction(state, instruction.get(), output);
}