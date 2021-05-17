#include "lmnt/compiler.hpp"
#include "lmnt/compiler_state.hpp"

#include <algorithm>
#include <vector>
#include <unordered_map>

#define U16_LO(x) static_cast<uint16_t>((x) & 0xFFFF)
#define U16_HI(x) static_cast<uint16_t>(((x) >> 16) & 0xFFFF)


element_result create_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** result = nullptr);

element_result prepare_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** result = nullptr);

element_result allocate_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** result = nullptr);

element_result compile_instruction(
    compiler_state& state,
    const element::instruction* expr,
    std::vector<lmnt_instruction>& output);


element_result copy_stack_values(const uint16_t src_index, const uint16_t dst_index, const uint16_t count, std::vector<lmnt_instruction>& output)
{
    if (src_index != dst_index)
    {
        const uint16_t countm4 = (count/4)*4;
        for (uint16_t i = 0; i < countm4; i += 4)
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNVV, uint16_t(src_index + i), 0, uint16_t(dst_index + i)});
        for (uint16_t i = countm4; i < count; ++i)
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNSS, uint16_t(src_index + i), 0, uint16_t(dst_index + i)});
    }
    return ELEMENT_OK;
}


//
// Constant
//

static element_result add_candidate_constant(compiler_state& state, element_value value)
{
    state.candidate_constants.try_emplace(value, 0);
    if (++state.candidate_constants[value] >= state.ctx.optimise.constant_reuse_threshold)
        return state.add_constant(value);
    return ELEMENT_OK;
}

static element_result create_virtual_constant(
    compiler_state& state,
    const element::instruction_constant& ec)
{
    ELEMENT_OK_OR_RETURN(state.allocator->set_count(&ec, 1));
    return add_candidate_constant(state, ec.value());
}

static element_result prepare_virtual_constant(
    compiler_state& state,
    const element::instruction_constant& ec)
{
    const bool is_output = state.allocator->is_type(&ec, allocation_type::output);
    uint16_t index;
    if (!is_output && state.find_constant(ec.value(), index) == ELEMENT_OK) {
        // hard-constant, just use it as-is
        ELEMENT_OK_OR_RETURN(state.allocator->set_pinned(&ec, allocation_type::constant, index, 1));
    }
    // else we have to copy it
    return ELEMENT_OK;
}

static element_result allocate_virtual_constant(
    compiler_state& state,
    const element::instruction_constant& ec)
{
    return state.allocator->allocate(&ec);
}

static element_result compile_constant_value(
    compiler_state& state,
    element_value value,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    uint16_t constant_idx;
    if (state.find_constant(value, constant_idx) == ELEMENT_OK) {
        // this constant is present in the "hard" constants list
        copy_stack_values(constant_idx, stack_idx, 1, output);
    } else {
        // encode the constant as part of the instruction
        uint32_t arg;
        assert(sizeof(arg) == sizeof(value));
        std::memcpy(&arg, &value, sizeof(value));
        output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIBS, U16_LO(arg), U16_HI(arg), stack_idx});
    }
    return ELEMENT_OK;
}

static element_result compile_constant(
    compiler_state& state,
    const element::instruction_constant& ec,
    const uint16_t outidx,
    std::vector<lmnt_instruction>& output)
{
    return compile_constant_value(state, ec.value(), outidx, output);
}


//
// Input
//

static element_result create_virtual_input(
    compiler_state& state,
    const element::instruction_input& ei)
{
    return state.allocator->set_count(&ei, 1);
}

static element_result prepare_virtual_input(
    compiler_state& state,
    const element::instruction_input& ei)
{
    if (!state.allocator->is_type(&ei, allocation_type::output)) {
        // just use it from the input location
        ELEMENT_OK_OR_RETURN(state.allocator->set_pinned(&ei, allocation_type::input, uint16_t(ei.index()), 1));
    }
    // else input --> output, have to copy it
    return ELEMENT_OK;
}

static element_result allocate_virtual_input(
    compiler_state& state,
    const element::instruction_input& ei)
{
    return state.allocator->allocate(&ei);
}

static element_result compile_input(
    compiler_state& state,
    const element::instruction_input& ei,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    copy_stack_values(state.calculate_stack_index(allocation_type::input, uint16_t(ei.index())), stack_idx, 1, output);
    return ELEMENT_OK;
}


//
// Serialised structure
//

static element_result create_virtual_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es)
{
    uint16_t count = 0;
    for (const auto& d : es.dependents())
    {
        stack_allocation* vr;
        ELEMENT_OK_OR_RETURN(create_virtual_result(state, d.get(), &vr));
        count += vr->count;
    }
    return state.allocator->set_count(&es, count);
}

static element_result prepare_virtual_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es)
{
    uint16_t index = 0;
    for (const auto& d : es.dependents())
    {
        ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, d.get()));
        // TODO: handle failure (copy in?)
        state.allocator->set_parent(d.get(), &es, index);
        state.allocator->use(&es, d.get());

        stack_allocation* d_vr = state.allocator->get(d.get());
        if (!d_vr) return ELEMENT_ERROR_UNKNOWN;
        index += d_vr->count;
    }

    return ELEMENT_OK;
}

static element_result allocate_virtual_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es)
{
    for (const auto& d : es.dependents())
        ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, d.get()));
    return state.allocator->allocate(&es);
}

static element_result compile_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es,
    const uint16_t stack_idx,
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

static element_value get_nullary_constant(element::instruction_nullary::op op)
{
    switch (op)
    {
    //num
    case element::instruction_nullary::op::positive_infinity:
        return std::numeric_limits<float>::infinity();
    case element::instruction_nullary::op::negative_infinity:
        return -std::numeric_limits<float>::infinity();
    case element::instruction_nullary::op::nan:
        return std::numeric_limits<float>::quiet_NaN();

    //boolean
    case element::instruction_nullary::op::true_value:
        return 1;
    case element::instruction_nullary::op::false_value:
        return 0;
    default:
        assert(false);
        return std::numeric_limits<float>::signaling_NaN();
    }
}

static element_result create_virtual_nullary(
    compiler_state& state,
    const element::instruction_nullary& en)
{
    ELEMENT_OK_OR_RETURN(state.allocator->set_count(&en, 1));

    // make this value eligible for hard-const
    element_value value = get_nullary_constant(en.operation());
    add_candidate_constant(state, value);

    return ELEMENT_OK;
}

static element_result prepare_virtual_nullary(
    compiler_state& state,
    const element::instruction_nullary& en)
{
    const bool is_output = state.allocator->is_type(&en, allocation_type::output);
    uint16_t index;
    if (!is_output && state.find_constant(get_nullary_constant(en.operation()), index) == ELEMENT_OK) {
        // hard-constant, just use it as-is
        ELEMENT_OK_OR_RETURN(state.allocator->set_pinned(&en, allocation_type::constant, index, 1));
    }
    return ELEMENT_OK;
}

static element_result allocate_virtual_nullary(
    compiler_state& state,
    const element::instruction_nullary& en)
{
    return state.allocator->allocate(&en);
}

static element_result compile_nullary(
    compiler_state& state,
    const element::instruction_nullary& en,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    element_value value = get_nullary_constant(en.operation());
    return compile_constant_value(state, value, stack_idx, output);
}


//
// Unary
//

static element_result create_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu)
{
    ELEMENT_OK_OR_RETURN(state.allocator->set_count(&eu, 1));

    if (eu.dependents().size() != 1)
        return ELEMENT_ERROR_UNKNOWN;

    stack_allocation* arg_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eu.dependents()[0].get(), &arg_vr));
    if (arg_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    // if we're doing boolean not we need to bring a couple of constants along
    if (eu.operation() == element::instruction_unary::op::not_)
    {
        ELEMENT_OK_OR_RETURN(state.add_constant(-1));
        ELEMENT_OK_OR_RETURN(state.add_constant(1));
    }

    return ELEMENT_OK;
}

static element_result prepare_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu)
{
    const element::instruction* dep0 = eu.dependents()[0].get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    state.allocator->use(&eu, dep0);
    return ELEMENT_OK;
}

static element_result allocate_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, eu.dependents()[0].get()));
    return state.allocator->allocate(&eu);
}

static element_result compile_unary(
    compiler_state& state,
    const element::instruction_unary& eu,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    const element::instruction* arg_in = eu.dependents()[0].get();
    // get the argument and ensure it's only 1 wide
    uint16_t arg_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(arg_in, arg_stack_idx));
    // compile the argument
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg_in, output));

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
        output.emplace_back(lmnt_instruction{LMNT_OP_CEILS, arg_stack_idx, 0, stack_idx});
        output.emplace_back(lmnt_instruction{LMNT_OP_MINSS, stack_idx, const_plus1, stack_idx});
        output.emplace_back(lmnt_instruction{LMNT_OP_MULSS, stack_idx, const_minus1, stack_idx});
        output.emplace_back(lmnt_instruction{LMNT_OP_ADDSS, stack_idx, const_plus1, stack_idx});
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
        output.emplace_back(lmnt_instruction{op, arg_stack_idx, 0, stack_idx});
    }
    return ELEMENT_OK;
}


//
// Binary
//

static element_result create_virtual_binary(
    compiler_state& state,
    const element::instruction_binary& eb)
{
    ELEMENT_OK_OR_RETURN(state.allocator->set_count(&eb, 1));

    stack_allocation* arg1_vr;
    stack_allocation* arg2_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eb.dependents()[0].get(), &arg1_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eb.dependents()[1].get(), &arg2_vr));
    if (arg1_vr->count != 1 || arg2_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    return ELEMENT_OK;
}

static element_result prepare_virtual_binary(
    compiler_state& state,
    const element::instruction_binary& eb)
{
    const element::instruction* dep0 = eb.dependents()[0].get();
    const element::instruction* dep1 = eb.dependents()[1].get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep1));
    state.allocator->use(&eb, dep0);
    state.allocator->use(&eb, dep1);
    return ELEMENT_OK;
}

static element_result allocate_virtual_binary(
    compiler_state& state,
    const element::instruction_binary& eb)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, eb.dependents()[0].get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, eb.dependents()[1].get()));
    return state.allocator->allocate(&eb);
}

static element_result compile_binary(
    compiler_state& state,
    const element::instruction_binary& eb,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    if (eb.dependents().size() != 2)
        return ELEMENT_ERROR_UNKNOWN;
    // get the arguments and ensure they're only 1 wide
    const element::instruction* arg1_in = eb.dependents()[0].get();
    const element::instruction* arg2_in = eb.dependents()[1].get();
    uint16_t arg1_stack_idx, arg2_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(arg1_in, arg1_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(arg2_in, arg2_stack_idx));
    // compile the arguments
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg1_in, output));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg2_in, output));

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
            output.emplace_back(lmnt_instruction{op, arg1_stack_idx, arg2_stack_idx, stack_idx});
            return ELEMENT_OK;

        case element::instruction_binary::op::log:
            // TODO: check if we've got a constant base we can work with
            output.emplace_back(lmnt_instruction{LMNT_OP_LOG, arg1_stack_idx, arg2_stack_idx, stack_idx});
            return ELEMENT_OK;

        default:
            break;
    }

    const uint32_t start_idx = static_cast<uint32_t>(output.size()); // index of the next op

    switch (eb.operation())
    {
        //boolean
        case element::instruction_binary::op::and_:
            // (arg1 > 0.0 && arg2 > 0.0)
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg1_stack_idx, 0, 0});                                 // + 0
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 1
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg2_stack_idx, 0, 0});                                 // + 2
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 3
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 1, 0, stack_idx});                                 // + 4
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH,    0, U16_LO(start_idx + 7), U16_HI(start_idx + 7)}); // + 5
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 0, 0, stack_idx});                                 // + 6
            return ELEMENT_OK;
        case element::instruction_binary::op::or_:
            // (arg1 > 0.0 || arg2 > 0.0)
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg1_stack_idx, 0, 0});                                 // + 0
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCGT, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 1
            output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, arg2_stack_idx, 0, 0});                                 // + 2
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCGT, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6)}); // + 3
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 0, 0, stack_idx});                                 // + 4
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, U16_LO(start_idx + 7), U16_HI(start_idx + 7)});    // + 5
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 1, 0, stack_idx});                                 // + 6
            return ELEMENT_OK;

        //comparison
        case element::instruction_binary::op::eq:  op = LMNT_OP_BRANCHCEQ; goto cmp_operation;
        case element::instruction_binary::op::neq: op = LMNT_OP_BRANCHCNE; goto cmp_operation;
        case element::instruction_binary::op::lt:  op = LMNT_OP_BRANCHCLT; goto cmp_operation;
        case element::instruction_binary::op::leq: op = LMNT_OP_BRANCHCLE; goto cmp_operation;
        case element::instruction_binary::op::gt:  op = LMNT_OP_BRANCHCGT; goto cmp_operation;
        case element::instruction_binary::op::geq: op = LMNT_OP_BRANCHCGE; goto cmp_operation;
    cmp_operation:
            output.emplace_back(lmnt_instruction{LMNT_OP_CMP, arg1_stack_idx, arg2_stack_idx, 0});                  // + 0
            output.emplace_back(lmnt_instruction{op, 0, U16_LO(start_idx + 4), U16_HI(start_idx + 4)});             // + 1
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 0, 0, stack_idx});                              // + 2
            output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, U16_LO(start_idx + 5), U16_HI(start_idx + 5)}); // + 3
            output.emplace_back(lmnt_instruction{LMNT_OP_ASSIGNIIS, 1, 0, stack_idx});                              // + 4
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
    const element::instruction_if& ei)
{
    if (ei.dependents().size() != 3)
        return ELEMENT_ERROR_UNKNOWN;

    // ensure the predicate is only 1 wide
    stack_allocation* predicate_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.dependents()[0].get(), &predicate_vr));
    if (predicate_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    // ensure both results of the if are at least the same size
    stack_allocation* true_vr;
    stack_allocation* false_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.dependents()[1].get(), &true_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.dependents()[2].get(), &false_vr));
    if (true_vr->count != false_vr->count)
        return ELEMENT_ERROR_UNKNOWN;

    return state.allocator->set_count(&ei, true_vr->count);
}

static element_result prepare_virtual_if(
    compiler_state& state,
    const element::instruction_if& ei)
{
    const element::instruction* dep0 = ei.dependents()[0].get();
    const element::instruction* dep1 = ei.dependents()[1].get();
    const element::instruction* dep2 = ei.dependents()[2].get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep1));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep2));
    state.allocator->use(&ei, dep0);
    // TODO: account for failure (copy in?)
    state.allocator->set_parent(dep1, &ei, 0);
    state.allocator->set_parent(dep2, &ei, 0);
    return ELEMENT_OK;
}

static element_result allocate_virtual_if(
    compiler_state& state,
    const element::instruction_if& ei)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.dependents()[0].get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.dependents()[1].get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.dependents()[2].get()));
    return state.allocator->allocate(&ei);
}

static element_result compile_if(
    compiler_state& state,
    const element::instruction_if& ei,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    const element::instruction* predicate_in = ei.dependents()[0].get();
    const element::instruction* true_in      = ei.dependents()[1].get();
    const element::instruction* false_in     = ei.dependents()[2].get();
    stack_allocation* true_vr = state.allocator->get(true_in);
    stack_allocation* false_vr = state.allocator->get(false_in);
    if (!true_vr || !false_vr) return ELEMENT_ERROR_UNKNOWN;

    uint16_t predicate_stack_idx, true_stack_idx, false_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(predicate_in, predicate_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(true_in, true_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(false_in, false_stack_idx));

    // compile the predicate
    ELEMENT_OK_OR_RETURN(compile_instruction(state, predicate_in, output));
    // make branch logic
    output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, predicate_stack_idx, 0, 0});
    const size_t predicate_branchcle_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, 0, 0}); // target filled in at the end
    // compile the true/false branches
    // TODO: hoist out anything in the true/false branches that's required elsewhere?
    ELEMENT_OK_OR_RETURN(compile_instruction(state, true_in, output));
    copy_stack_values(true_stack_idx, stack_idx, true_vr->count, output);
    const size_t branch_past_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCH, 0, 0, 0}); // target filled in at the end
    const size_t false_idx = output.size();
    ELEMENT_OK_OR_RETURN(compile_instruction(state, false_in, output));
    copy_stack_values(false_stack_idx, stack_idx, false_vr->count, output);
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
    const element::instruction_for& ef)
{
    if (ef.dependents().size() != 3)
        return ELEMENT_ERROR_UNKNOWN;

    stack_allocation* initial_vr;
    stack_allocation* condition_vr;
    stack_allocation* body_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.dependents()[0].get(), &initial_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.dependents()[1].get(), &condition_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.dependents()[2].get(), &body_vr));
    if (condition_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;
    if (initial_vr->count != body_vr->count)
        return ELEMENT_ERROR_UNKNOWN;

    return state.allocator->set_count(&ef, initial_vr->count);
}

static element_result prepare_virtual_for(
    compiler_state& state,
    const element::instruction_for& ef)
{
    const element::instruction* dep0 = ef.dependents()[0].get();
    const element::instruction* dep1 = ef.dependents()[1].get();
    const element::instruction* dep2 = ef.dependents()[2].get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep1));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep2));

    state.allocator->set_parent(dep0, &ef, 0);
    state.allocator->set_parent(dep2, &ef, 0);

    state.allocator->use(&ef, dep1);
    return ELEMENT_OK;
}

static element_result allocate_virtual_for(
    compiler_state& state,
    const element::instruction_for& ef)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ef.dependents()[0].get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ef.dependents()[1].get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ef.dependents()[2].get()));
    return state.allocator->allocate(&ef);
}

static element_result compile_for(
    compiler_state& state,
    const element::instruction_for& ef,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    const element::instruction* initial_in = ef.dependents()[0].get();
    const element::instruction* condition_in = ef.dependents()[1].get();
    const element::instruction* body_in = ef.dependents()[2].get();
    stack_allocation* initial_vr = state.allocator->get(initial_in);
    stack_allocation* condition_vr = state.allocator->get(condition_in);
    stack_allocation* body_vr = state.allocator->get(body_in);
    if (!initial_vr || !condition_vr || !body_vr) return ELEMENT_ERROR_UNKNOWN;

    uint16_t initial_stack_idx, condition_stack_idx, body_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(initial_in, initial_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(condition_in, condition_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(body_in, body_stack_idx));

    // compile the initial state
    ELEMENT_OK_OR_RETURN(compile_instruction(state, initial_in, output));
    copy_stack_values(initial_stack_idx, stack_idx, initial_vr->count, output);
    // compile condition logic
    const size_t condition_index = output.size();
    ELEMENT_OK_OR_RETURN(compile_instruction(state, condition_vr->instruction, output));
    output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, condition_stack_idx, 0, 0});
    const size_t condition_branchcle_idx = output.size();
    output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCLE, 0, 0, 0}); // target filled in at the end
    // compile the loop body
    ELEMENT_OK_OR_RETURN(compile_instruction(state, body_vr->instruction, output));
    copy_stack_values(body_stack_idx, stack_idx, body_vr->count, output);
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
    const element::instruction_indexer& ei)
{
    stack_allocation* for_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.for_instruction().get(), &for_vr));
    if (ei.index >= for_vr->count)
        return ELEMENT_ERROR_UNKNOWN;

    return state.allocator->set_count(&ei, 1);
}

static element_result prepare_virtual_indexer(
    compiler_state& state,
    const element::instruction_indexer& ei)
{
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, ei.for_instruction().get()));
    state.allocator->use(&ei, ei.for_instruction().get());
    // TODO: verify this is a good idea
    state.allocator->set_parent(&ei, ei.for_instruction().get(), ei.index);
    return ELEMENT_OK;
}

static element_result allocate_virtual_indexer(
    compiler_state& state,
    const element::instruction_indexer& ei)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.for_instruction().get()));
    return state.allocator->allocate(&ei);
}

static element_result compile_indexer(
    compiler_state& state,
    const element::instruction_indexer& ei,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    ELEMENT_OK_OR_RETURN(compile_instruction(state, ei.for_instruction().get(), output));

    uint16_t for_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(ei.for_instruction().get(), for_stack_idx));

    const uint16_t for_entry_idx = static_cast<uint16_t>(for_stack_idx + ei.index);
    copy_stack_values(for_entry_idx, stack_idx, 1, output);
    return ELEMENT_OK;
}


//
// Select
//

static element_result create_virtual_select(
    compiler_state& state,
    const element::instruction_select& es)
{
    stack_allocation* selector_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, es.selector().get(), &selector_vr));
    if (selector_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    if (es.options_count() == 0)
        return ELEMENT_ERROR_UNKNOWN;

    ELEMENT_OK_OR_RETURN(state.add_constant(0));
    ELEMENT_OK_OR_RETURN(state.add_constant(1));
    ELEMENT_OK_OR_RETURN(state.add_constant(element_value(es.options_count() - 1)));

    uint16_t max_count = 0;
    uint16_t index = 0;
    for (size_t i = 0; i < es.options_count(); ++i)
    {
        stack_allocation* option_vr;
        ELEMENT_OK_OR_RETURN(create_virtual_result(state, es.options_at(i).get(), &option_vr));
        max_count = (std::max)(max_count, static_cast<uint16_t>(option_vr->count));
    }

    return state.allocator->set_count(&es, max_count);
}

static element_result prepare_virtual_select(
    compiler_state& state,
    const element::instruction_select& es)
{
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, es.selector().get()));
    state.allocator->use(&es, es.selector().get());
    const size_t opts_size = es.options_count();
    for (size_t i = 0; i < opts_size; ++i)
    {
        ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, es.options_at(i).get()));
        // TODO: handle failure (copy in?)
        state.allocator->set_parent(es.options_at(i).get(), &es, 0);
    }
    return ELEMENT_OK;
}

static element_result allocate_virtual_select(
    compiler_state& state,
    const element::instruction_select& es)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, es.selector().get()));
    const size_t opts_size = es.options_count();
    for (size_t i = 0; i < opts_size; ++i)
        ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, es.options_at(i).get()));
    return state.allocator->allocate(&es);
}

static element_result compile_select(
    compiler_state& state,
    const element::instruction_select& es,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output)
{
    const size_t opts_size = es.options_count();
    uint16_t zero_idx, one_idx, last_valid_idx;
    ELEMENT_OK_OR_RETURN(state.find_constant(0, zero_idx));
    ELEMENT_OK_OR_RETURN(state.find_constant(1, one_idx));
    ELEMENT_OK_OR_RETURN(state.find_constant(element_value(opts_size - 1), last_valid_idx));

    stack_allocation* selector_vr = state.allocator->get(es.selector().get());
    if (!selector_vr) return ELEMENT_ERROR_UNKNOWN;

    uint16_t selector_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(es.selector().get(), selector_stack_idx));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, es.selector().get(), output));
    // clamp in range and ensure it's an integer
    // note we can't use the selector stack index here as it could be anything (a constant, an input...)
    output.emplace_back(lmnt_instruction{LMNT_OP_MAXSS,  selector_stack_idx, zero_idx, stack_idx});
    output.emplace_back(lmnt_instruction{LMNT_OP_MINSS,  stack_idx, last_valid_idx, stack_idx});
    output.emplace_back(lmnt_instruction{LMNT_OP_TRUNCS, stack_idx, 0, stack_idx});

    const size_t branches_start_idx = output.size();
    for (size_t i = 0; i < opts_size; ++i)
    {
        output.emplace_back(lmnt_instruction{LMNT_OP_CMPZ, stack_idx, 0, 0});
        output.emplace_back(lmnt_instruction{LMNT_OP_BRANCHCEQ, 0, 0, 0}); // target filled in later
        output.emplace_back(lmnt_instruction{LMNT_OP_SUBSS, stack_idx, one_idx, stack_idx});
    }

    uint16_t option0_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(es.options_at(0).get(), option0_stack_idx));
    const bool stacked = (option0_stack_idx == stack_idx);

    std::vector<size_t> option_branch_indexes(opts_size);

    for (size_t i = 0; i < opts_size; ++i)
    {
        stack_allocation* option_vr = state.allocator->get(es.options_at(i).get());
        if (!option_vr) return ELEMENT_ERROR_UNKNOWN;

        uint16_t option_stack_idx;
        ELEMENT_OK_OR_RETURN(state.calculate_stack_index(es.options_at(i).get(), option_stack_idx));

        const size_t option_idx = output.size();
        // fill in the target index for this branch option
        output[branches_start_idx + i*3 + 1].arg2 = U16_LO(option_idx);
        output[branches_start_idx + i*3 + 1].arg3 = U16_HI(option_idx);
        ELEMENT_OK_OR_RETURN(compile_instruction(state, es.options_at(i).get(), output));
        copy_stack_values(option_stack_idx, stack_idx, option_vr->count, output);
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
    stack_allocation** vr)
{
    // CSE: if this is non-null, we already did this
    stack_allocation* existing_vr = state.allocator->get(expr);
    if (existing_vr)
    {
        if (vr) *vr = existing_vr;
        return ELEMENT_OK;
    }

    // fake instruction index for now
    ELEMENT_OK_OR_RETURN(state.allocator->add(expr, 0, 0, &existing_vr));

    element_result oresult = ELEMENT_ERROR_NO_IMPL;

    if (const auto* ec = expr->as<element::instruction_constant>())
        oresult = create_virtual_constant(state, *ec);

    if (const auto* ei = expr->as<element::instruction_input>())
        oresult = create_virtual_input(state, *ei);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        oresult = create_virtual_serialised_structure(state, *es);

    if (const auto* en = expr->as<element::instruction_nullary>())
        oresult = create_virtual_nullary(state, *en);

    if (const auto* eu = expr->as<element::instruction_unary>())
        oresult = create_virtual_unary(state, *eu);

    if (const auto* eb = expr->as<element::instruction_binary>())
        oresult = create_virtual_binary(state, *eb);

    if (const auto* ei = expr->as<element::instruction_if>())
        oresult = create_virtual_if(state, *ei);

    if (const auto* ef = expr->as<element::instruction_for>())
        oresult = create_virtual_for(state, *ef);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        oresult = create_virtual_indexer(state, *ei);

    if (const auto* sel = expr->as<element::instruction_select>())
        oresult = create_virtual_select(state, *sel);

    if (oresult == ELEMENT_OK)
    {
        existing_vr->stage = compilation_stage::created;
        existing_vr->set_instruction = state.cur_instruction_index;
        existing_vr->last_used_instruction = state.cur_instruction_index;
        if (vr) *vr = existing_vr;
    }

    ++state.cur_instruction_index;
    return oresult;
}


static element_result prepare_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** vr)
{
    // CSE: if this is non-null, we already did this
    stack_allocation* existing_vr = state.allocator->get(expr);
    if (!existing_vr) return ELEMENT_ERROR_UNKNOWN;

    if (existing_vr->stage >= compilation_stage::prepared)
    {
        if (vr) *vr = existing_vr;
        return ELEMENT_OK;
    }

    element_result oresult = ELEMENT_ERROR_NO_IMPL;

    if (const auto* ec = expr->as<element::instruction_constant>())
        oresult = prepare_virtual_constant(state, *ec);

    if (const auto* ei = expr->as<element::instruction_input>())
        oresult = prepare_virtual_input(state, *ei);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        oresult = prepare_virtual_serialised_structure(state, *es);

    if (const auto* en = expr->as<element::instruction_nullary>())
        oresult = prepare_virtual_nullary(state, *en);

    if (const auto* eu = expr->as<element::instruction_unary>())
        oresult = prepare_virtual_unary(state, *eu);

    if (const auto* eb = expr->as<element::instruction_binary>())
        oresult = prepare_virtual_binary(state, *eb);

    if (const auto* ei = expr->as<element::instruction_if>())
        oresult = prepare_virtual_if(state, *ei);

    if (const auto* ef = expr->as<element::instruction_for>())
        oresult = prepare_virtual_for(state, *ef);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        oresult = prepare_virtual_indexer(state, *ei);

    if (const auto* sel = expr->as<element::instruction_select>())
        oresult = prepare_virtual_select(state, *sel);

    if (oresult == ELEMENT_OK)
    {
        existing_vr->stage = compilation_stage::prepared;
        if (vr) *vr = existing_vr;
    }

    return oresult;
}


static element_result allocate_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** vr)
{
    stack_allocation* existing_vr = state.allocator->get(expr);
    if (!existing_vr) return ELEMENT_ERROR_UNKNOWN;

    if (existing_vr->stage >= compilation_stage::allocated)
    {
        if (vr) *vr = existing_vr;
        return ELEMENT_OK;
    }

    element_result oresult = ELEMENT_ERROR_NO_IMPL;

    if (const auto* ec = expr->as<element::instruction_constant>())
        oresult = allocate_virtual_constant(state, *ec);

    if (const auto* ei = expr->as<element::instruction_input>())
        oresult = allocate_virtual_input(state, *ei);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        oresult = allocate_virtual_serialised_structure(state, *es);

    if (const auto* en = expr->as<element::instruction_nullary>())
        oresult = allocate_virtual_nullary(state, *en);

    if (const auto* eu = expr->as<element::instruction_unary>())
        oresult = allocate_virtual_unary(state, *eu);

    if (const auto* eb = expr->as<element::instruction_binary>())
        oresult = allocate_virtual_binary(state, *eb);

    if (const auto* ei = expr->as<element::instruction_if>())
        oresult = allocate_virtual_if(state, *ei);

    if (const auto* ef = expr->as<element::instruction_for>())
        oresult = allocate_virtual_for(state, *ef);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        oresult = allocate_virtual_indexer(state, *ei);

    if (const auto* sel = expr->as<element::instruction_select>())
        oresult = allocate_virtual_select(state, *sel);

    if (oresult == ELEMENT_OK)
    {
        existing_vr->stage = compilation_stage::allocated;
        if (vr) *vr = existing_vr;
    }

    return oresult;
}


static element_result compile_instruction(
    compiler_state& state,
    const element::instruction* expr,
    std::vector<lmnt_instruction>& output)
{
    stack_allocation* vr = state.allocator->get(expr);
    if (!vr) return ELEMENT_ERROR_UNKNOWN;

    if (vr->stage >= compilation_stage::compiled)
        return ELEMENT_OK;

    uint16_t index;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(expr, index));

    element_result oresult = ELEMENT_ERROR_NO_IMPL;

    if (const auto* ec = expr->as<element::instruction_constant>())
        oresult = compile_constant(state, *ec, index, output);

    if (const auto* ei = expr->as<element::instruction_input>())
        oresult = compile_input(state, *ei, index, output);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        oresult = compile_serialised_structure(state, *es, index, output);

    if (const auto* en = expr->as<element::instruction_nullary>())
        oresult = compile_nullary(state, *en, index, output);

    if (const auto* eu = expr->as<element::instruction_unary>())
        oresult = compile_unary(state, *eu, index, output);

    if (const auto* eb = expr->as<element::instruction_binary>())
        oresult = compile_binary(state, *eb, index, output);

    if (const auto* ei = expr->as<element::instruction_if>())
        oresult = compile_if(state, *ei, index, output);

    if (const auto* ef = expr->as<element::instruction_for>())
        oresult = compile_for(state, *ef, index, output);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        oresult = compile_indexer(state, *ei, index, output);

    if (const auto* sel = expr->as<element::instruction_select>())
        oresult = compile_select(state, *sel, index, output);

    if (oresult == ELEMENT_OK)
        vr->stage = compilation_stage::compiled;

    return oresult;
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
    std::vector<element_value>& constants,
    const size_t inputs_count,
    element_lmnt_compiled_function& output)
{
    compiler_state state { ctx, instruction.get(), constants, static_cast<uint16_t>(inputs_count) };
    // TODO: check for single constant fast path
    stack_allocation* vr = nullptr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, instruction.get(), &vr));
    // set ourselves as being an output
    ELEMENT_OK_OR_RETURN(state.allocator->set_pinned(instruction.get(), allocation_type::output, 0, vr->count));
    // continue with compilation
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, instruction.get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, instruction.get()));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, instruction.get(), output.instructions));
    output.inputs_count = inputs_count;

    output.outputs_count = vr->count;
    output.local_stack_count = state.allocator->get_max_stack_usage();
    return ELEMENT_OK;
}