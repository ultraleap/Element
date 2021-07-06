#include "lmnt/compiler.hpp"
#include "lmnt/compiler_state.hpp"

#include <algorithm>
#include <vector>
#include <unordered_map>

#define U16_LO(x) static_cast<uint16_t>((x)&0xFFFF)
#define U16_HI(x) static_cast<uint16_t>(((x) >> 16) & 0xFFFF)

// create an allocation for this instruction and its dependents
// must know what size the allocation is
static element_result create_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** result = nullptr);

// determine relationships between this instruction and its dependents
// pinned and parenting statuses get set here
// also take note of which instructions use data from other instructions
static element_result prepare_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** result = nullptr);

// decide how stack space is allocated for this instruction and its dependents
static element_result allocate_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** result = nullptr);

// generate LMNT bytecode for instruction and its dependents
static element_result compile_instruction(
    compiler_state& state,
    const element::instruction* expr,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags);

static element_result copy_stack_values(const uint16_t src_index, const uint16_t dst_index, const uint16_t count, std::vector<lmnt_instruction>& output)
{
    if (src_index != dst_index) {
        const uint16_t countm4 = (count / 4) * 4;
        for (uint16_t i = 0; i < countm4; i += 4)
            output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNVV, uint16_t(src_index + i), 0, uint16_t(dst_index + i) });
        for (uint16_t i = countm4; i < count; ++i)
            output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNSS, uint16_t(src_index + i), 0, uint16_t(dst_index + i) });
    }
    return ELEMENT_OK;
}

// determine what leaf instruction owns the allocation at index N within the given instruction
static const element::instruction* instruction_at(const compiler_state& state, const element::instruction* expr, size_t index, size_t count)
{
    if (const auto* es = expr->as<element::instruction_serialised_structure>()) {
        size_t st_index = 0;
        for (const auto& d : es->dependents()) {
            const stack_allocation* alloc = state.allocator->get(d.get());
            if (!alloc)
                return nullptr;

            if (st_index >= index)
                return instruction_at(state, d.get(), st_index - index, count);
            st_index += alloc->count;
        }
        return nullptr;
    } else if (const auto* ef = expr->as<element::instruction_for>()) {
        return instruction_at(state, ef->body().get(), index, count);
    } else if (const auto* ei = expr->as<element::instruction_indexer>()) {
        return instruction_at(state, ei->for_instruction().get(), index + ei->index, count);
    } else {
        const stack_allocation* alloc = state.allocator->get(expr);
        if (!alloc)
            return nullptr;

        return (index + count <= alloc->count) ? expr : nullptr;
    }
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
    ELEMENT_OK_OR_RETURN(state.allocator->add(&ec, 1));
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
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
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
        output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNIBS, U16_LO(arg), U16_HI(arg), stack_idx });
    }
    return ELEMENT_OK;
}

static element_result compile_constant(
    compiler_state& state,
    const element::instruction_constant& ec,
    const uint16_t outidx,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    return compile_constant_value(state, ec.value(), outidx, output, flags);
}

//
// Input
//

static element_result create_virtual_input(
    compiler_state& state,
    const element::instruction_input& ei)
{
    return state.allocator->add(&ei, 1);
}

static element_result prepare_virtual_input(
    compiler_state& state,
    const element::instruction_input& ei)
{
    // check if this is a top-level input (scope 0)
    const bool top_level = (ei.scope() == 0);
    if (top_level && !state.allocator->is_type(&ei, allocation_type::output)) {
        // just use it from the input location
        ELEMENT_OK_OR_RETURN(state.allocator->set_pinned(&ei, allocation_type::input, uint16_t(ei.index()), 1));
    }
    // else input --> output or input to local function, so have to copy it
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
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    const bool top_level = (ei.scope() == 0);
    if (top_level)
        copy_stack_values(state.calculate_stack_index(allocation_type::input, uint16_t(ei.index())), stack_idx, 1, output);
    // else filled in wherever the local function is invoked
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
    for (const auto& d : es.dependents()) {
        stack_allocation* vr;
        ELEMENT_OK_OR_RETURN(create_virtual_result(state, d.get(), &vr));
        count += vr->count;
    }
    return state.allocator->add(&es, count);
}

static element_result prepare_virtual_serialised_structure(
    compiler_state& state,
    const element::instruction_serialised_structure& es)
{
    uint16_t index = 0;
    for (const auto& d : es.dependents()) {
        ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, d.get()));
        state.allocator->set_parent(d.get(), &es, index);
        state.use(&es, d.get());

        stack_allocation* d_vr = state.allocator->get(d.get());
        if (!d_vr)
            return ELEMENT_ERROR_UNKNOWN;
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
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    uint16_t index = 0;
    for (const auto& d : es.dependents()) {
        ELEMENT_OK_OR_RETURN(compile_instruction(state, d.get(), output, flags));

        const stack_allocation* d_vr = state.allocator->get(d.get());
        if (!d_vr)
            return ELEMENT_ERROR_UNKNOWN;

        // in most cases this will be a no-op
        // however if the dependent is pinned, this will copy it into the struct's memory block
        // this is to ensure that structs always contain the values they're meant to
        uint16_t d_index = state.calculate_stack_index(d_vr->type(), d_vr->index());
        copy_stack_values(d_index, stack_idx + index, d_vr->count, output);
        index += d_vr->count;
    }
    return ELEMENT_OK;
}

//
// Nullary
//

static element_value get_nullary_constant(element::instruction_nullary::op op)
{
    switch (op) {
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
    ELEMENT_OK_OR_RETURN(state.allocator->add(&en, 1));

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
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    element_value value = get_nullary_constant(en.operation());
    return compile_constant_value(state, value, stack_idx, output, flags);
}

//
// Unary
//

static element_result create_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu)
{
    ELEMENT_OK_OR_RETURN(state.allocator->add(&eu, 1));

    stack_allocation* arg_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eu.input().get(), &arg_vr));
    if (arg_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    // if we're doing boolean not we need to bring a couple of constants along
    if (eu.operation() == element::instruction_unary::op::not_) {
        ELEMENT_OK_OR_RETURN(state.add_constant(-1));
        ELEMENT_OK_OR_RETURN(state.add_constant(1));
    }

    return ELEMENT_OK;
}

static element_result prepare_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu)
{
    const element::instruction* dep0 = eu.input().get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    state.use(&eu, dep0);
    return ELEMENT_OK;
}

static element_result allocate_virtual_unary(
    compiler_state& state,
    const element::instruction_unary& eu)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, eu.input().get()));
    return state.allocator->allocate(&eu);
}

static element_result compile_unary(
    compiler_state& state,
    const element::instruction_unary& eu,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    const element::instruction* arg_in = eu.input().get();
    // get the argument and ensure it's only 1 wide
    uint16_t arg_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(arg_in, arg_stack_idx));
    // compile the argument
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg_in, output, flags));

    if (eu.operation() == element::instruction_unary::op::not_) {
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
        output.emplace_back(lmnt_instruction{ LMNT_OP_CEILS, arg_stack_idx, 0, stack_idx });
        output.emplace_back(lmnt_instruction{ LMNT_OP_MINSS, stack_idx, const_plus1, stack_idx });
        output.emplace_back(lmnt_instruction{ LMNT_OP_MULSS, stack_idx, const_minus1, stack_idx });
        output.emplace_back(lmnt_instruction{ LMNT_OP_ADDSS, stack_idx, const_plus1, stack_idx });
    } else {
        lmnt_opcode op;
        switch (eu.operation()) {
        case element::instruction_unary::op::abs:
            op = LMNT_OP_ABSS;
            break;
        case element::instruction_unary::op::acos:
            op = LMNT_OP_ACOS;
            break;
        case element::instruction_unary::op::asin:
            op = LMNT_OP_ASIN;
            break;
        case element::instruction_unary::op::atan:
            op = LMNT_OP_ATAN;
            break;
        case element::instruction_unary::op::ceil:
            op = LMNT_OP_CEILS;
            break;
        case element::instruction_unary::op::cos:
            op = LMNT_OP_COS;
            break;
        case element::instruction_unary::op::floor:
            op = LMNT_OP_FLOORS;
            break;
        case element::instruction_unary::op::ln:
            op = LMNT_OP_LN;
            break;
        case element::instruction_unary::op::sin:
            op = LMNT_OP_SIN;
            break;
        case element::instruction_unary::op::tan:
            op = LMNT_OP_TAN;
            break;
        default:
            assert(false);
            return ELEMENT_ERROR_UNKNOWN;
        }
        output.emplace_back(lmnt_instruction{ op, arg_stack_idx, 0, stack_idx });
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
    ELEMENT_OK_OR_RETURN(state.allocator->add(&eb, 1));

    stack_allocation* arg1_vr;
    stack_allocation* arg2_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eb.input1().get(), &arg1_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, eb.input2().get(), &arg2_vr));
    if (arg1_vr->count != 1 || arg2_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    return ELEMENT_OK;
}

static element_result prepare_virtual_binary(
    compiler_state& state,
    const element::instruction_binary& eb)
{
    const element::instruction* dep0 = eb.input1().get();
    const element::instruction* dep1 = eb.input2().get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep1));
    state.use(&eb, dep0);
    state.use(&eb, dep1);
    return ELEMENT_OK;
}

static element_result allocate_virtual_binary(
    compiler_state& state,
    const element::instruction_binary& eb)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, eb.input1().get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, eb.input2().get()));
    return state.allocator->allocate(&eb);
}

static element_result compile_binary(
    compiler_state& state,
    const element::instruction_binary& eb,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    // get the arguments and ensure they're only 1 wide
    const element::instruction* arg1_in = eb.input1().get();
    const element::instruction* arg2_in = eb.input2().get();
    uint16_t arg1_stack_idx, arg2_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(arg1_in, arg1_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(arg2_in, arg2_stack_idx));
    // compile the arguments
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg1_in, output, flags));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, arg2_in, output, flags));

    lmnt_opcode op;
    switch (eb.operation()) {
    //num
    case element::instruction_binary::op::add:
        op = LMNT_OP_ADDSS;
        goto num_operation;
    case element::instruction_binary::op::atan2:
        op = LMNT_OP_ATAN2;
        goto num_operation;
    case element::instruction_binary::op::div:
        op = LMNT_OP_DIVSS;
        goto num_operation;
    case element::instruction_binary::op::max:
        op = LMNT_OP_MAXSS;
        goto num_operation;
    case element::instruction_binary::op::min:
        op = LMNT_OP_MINSS;
        goto num_operation;
    case element::instruction_binary::op::mul:
        op = LMNT_OP_MULSS;
        goto num_operation;
    case element::instruction_binary::op::pow:
        op = LMNT_OP_POWSS;
        goto num_operation;
    case element::instruction_binary::op::rem:
        op = LMNT_OP_MODSS;
        goto num_operation;
    case element::instruction_binary::op::sub:
        op = LMNT_OP_SUBSS;
        goto num_operation;
    num_operation:
        output.emplace_back(lmnt_instruction{ op, arg1_stack_idx, arg2_stack_idx, stack_idx });
        return ELEMENT_OK;

    case element::instruction_binary::op::log:
        // TODO: check if we've got a constant base we can work with
        output.emplace_back(lmnt_instruction{ LMNT_OP_LOG, arg1_stack_idx, arg2_stack_idx, stack_idx });
        return ELEMENT_OK;

    default:
        break;
    }

    const uint32_t start_idx = static_cast<uint32_t>(output.size()); // index of the next op

    switch (eb.operation()) {
    //boolean
    case element::instruction_binary::op::and_:
        // (arg1 > 0.0 && arg2 > 0.0)
        output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, arg1_stack_idx, 0, 0 });                                 // + 0
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCLE, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6) }); // + 1
        output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, arg2_stack_idx, 0, 0 });                                 // + 2
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCLE, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6) }); // + 3
        output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNIIS, 1, 0, stack_idx });                                 // + 4
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCH, 0, U16_LO(start_idx + 7), U16_HI(start_idx + 7) });    // + 5
        output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNIIS, 0, 0, stack_idx });                                 // + 6
        return ELEMENT_OK;
    case element::instruction_binary::op::or_:
        // (arg1 > 0.0 || arg2 > 0.0)
        output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, arg1_stack_idx, 0, 0 });                                 // + 0
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCGT, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6) }); // + 1
        output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, arg2_stack_idx, 0, 0 });                                 // + 2
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCGT, 0, U16_LO(start_idx + 6), U16_HI(start_idx + 6) }); // + 3
        output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNIIS, 0, 0, stack_idx });                                 // + 4
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCH, 0, U16_LO(start_idx + 7), U16_HI(start_idx + 7) });    // + 5
        output.emplace_back(lmnt_instruction{ LMNT_OP_ASSIGNIIS, 1, 0, stack_idx });                                 // + 6
        return ELEMENT_OK;

    //comparison
    case element::instruction_binary::op::eq:
        op = LMNT_OP_ASSIGNCEQ;
        goto cmp_operation;
    case element::instruction_binary::op::neq:
        op = LMNT_OP_ASSIGNCNE;
        goto cmp_operation;
    case element::instruction_binary::op::lt:
        op = LMNT_OP_ASSIGNCLT;
        goto cmp_operation;
    case element::instruction_binary::op::leq:
        op = LMNT_OP_ASSIGNCLE;
        goto cmp_operation;
    case element::instruction_binary::op::gt:
        op = LMNT_OP_ASSIGNCGT;
        goto cmp_operation;
    case element::instruction_binary::op::geq:
        op = LMNT_OP_ASSIGNCGE;
        goto cmp_operation;
    cmp_operation:
        output.emplace_back(lmnt_instruction{ LMNT_OP_CMP, arg1_stack_idx, arg2_stack_idx, 0 });
        output.emplace_back(lmnt_instruction{ op, 1, 0, stack_idx });
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
    // ensure the predicate is only 1 wide
    stack_allocation* predicate_vr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.predicate().get(), &predicate_vr));
    if (predicate_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;

    // ensure both results of the if are at least the same size
    stack_allocation* true_vr;
    stack_allocation* false_vr;

    ELEMENT_OK_OR_RETURN(state.push_context(ei.if_true().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.if_true().get(), &true_vr));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    ELEMENT_OK_OR_RETURN(state.push_context(ei.if_false().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ei.if_false().get(), &false_vr));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    if (true_vr->count != false_vr->count)
        return ELEMENT_ERROR_UNKNOWN;

    return state.allocator->add(&ei, true_vr->count);
}

static element_result prepare_virtual_if(
    compiler_state& state,
    const element::instruction_if& ei)
{
    const element::instruction* dep0 = ei.predicate().get();
    const element::instruction* dep1 = ei.if_true().get();
    const element::instruction* dep2 = ei.if_false().get();
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));

    ELEMENT_OK_OR_RETURN(state.push_context(dep1, execution_type::conditional));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep1));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    ELEMENT_OK_OR_RETURN(state.push_context(dep2, execution_type::conditional));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep2));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    // TODO: identify any subtrees appearing in both branches and mark them as unconditional *if* current context is
    // TODO: hoist anything unconditional in branch scopes out to our scope

    state.use(&ei, dep0);
    // TODO: account for failure (copy in?)
    state.allocator->set_parent(dep1, &ei, 0);
    state.allocator->set_parent(dep2, &ei, 0);
    return ELEMENT_OK;
}

static element_result allocate_virtual_if(
    compiler_state& state,
    const element::instruction_if& ei)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.predicate().get()));

    ELEMENT_OK_OR_RETURN(state.push_context(ei.if_true().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.if_true().get()));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    ELEMENT_OK_OR_RETURN(state.push_context(ei.if_false().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ei.if_false().get()));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    return state.allocator->allocate(&ei);
}

static element_result compile_if(
    compiler_state& state,
    const element::instruction_if& ei,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    const element::instruction* predicate_in = ei.predicate().get();
    const element::instruction* true_in = ei.if_true().get();
    const element::instruction* false_in = ei.if_false().get();
    stack_allocation* true_vr = state.allocator->get(true_in);
    stack_allocation* false_vr = state.allocator->get(false_in);
    if (!true_vr || !false_vr)
        return ELEMENT_ERROR_UNKNOWN;

    uint16_t predicate_stack_idx, true_stack_idx, false_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(predicate_in, predicate_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(true_in, true_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(false_in, false_stack_idx));

    // TODO: find anything unconditional in the true/false branches and compile it in case we're the first use

    // compile the predicate
    ELEMENT_OK_OR_RETURN(compile_instruction(state, predicate_in, output, flags));
    // make branch logic
    output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, predicate_stack_idx, 0, 0 });
    const size_t predicate_branchcle_idx = output.size();
    output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCLE, 0, 0, 0 }); // target filled in at the end
    // compile the true branch
    ELEMENT_OK_OR_RETURN(state.push_context(true_in, execution_type::conditional));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, true_in, output, flags));
    ELEMENT_OK_OR_RETURN(state.pop_context());
    copy_stack_values(true_stack_idx, stack_idx, true_vr->count, output);
    const size_t branch_past_idx = output.size();
    output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCH, 0, 0, 0 }); // target filled in at the end
    // compile the false branch
    const size_t false_idx = output.size();
    ELEMENT_OK_OR_RETURN(state.push_context(false_in, execution_type::conditional));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, false_in, output, flags));
    ELEMENT_OK_OR_RETURN(state.pop_context());
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
    stack_allocation* initial_vr;
    stack_allocation* condition_vr;
    stack_allocation* body_vr;

    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.initial().get(), &initial_vr));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.condition().get(), &condition_vr));

    ELEMENT_OK_OR_RETURN(state.push_context(ef.body().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, ef.body().get(), &body_vr));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    if (condition_vr->count != 1)
        return ELEMENT_ERROR_UNKNOWN;
    if (initial_vr->count != body_vr->count)
        return ELEMENT_ERROR_UNKNOWN;

    return state.allocator->add(&ef, initial_vr->count);
}

static element_result prepare_virtual_for(
    compiler_state& state,
    const element::instruction_for& ef)
{
    const element::instruction* dep0 = ef.initial().get();
    const element::instruction* dep1 = ef.condition().get();
    const element::instruction* dep2 = ef.body().get();

    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep0));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep1));

    ELEMENT_OK_OR_RETURN(state.push_context(dep2, execution_type::conditional));
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, dep2));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    // TODO: hoist out anything unconditional

    // these may fail if they're pinned, which is fine
    state.allocator->set_parent(dep0, &ef, 0);
    state.allocator->set_parent(dep2, &ef, 0);

    state.use(&ef, dep1);
    return ELEMENT_OK;
}

static element_result allocate_virtual_for(
    compiler_state& state,
    const element::instruction_for& ef)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ef.initial().get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ef.condition().get()));

    ELEMENT_OK_OR_RETURN(state.push_context(ef.body().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, ef.body().get()));
    ELEMENT_OK_OR_RETURN(state.pop_context());

    return state.allocator->allocate(&ef);
}

using input_set = std::set<std::shared_ptr<const element::instruction_input>>;
static element_result find_input_allocations(const compiler_state& state, const element::instruction* in, const input_set& expected, std::vector<const stack_allocation*>& result)
{
    const auto* input = in->as<element::instruction_input>();
    if (input && std::find_if(expected.begin(), expected.end(), [&](const auto& e) { return e.get() == input; }) != expected.end()) {
        if (result.size() <= input->index())
            result.resize(input->index() + 1, nullptr);
        result[input->index()] = state.allocator->get(input);
    }

    for (const auto& d : in->dependents())
        ELEMENT_OK_OR_RETURN(find_input_allocations(state, d.get(), expected, result));
    return ELEMENT_OK;
}

static element_result compile_for(
    compiler_state& state,
    const element::instruction_for& ef,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    const element::instruction* initial_in = ef.initial().get();
    const element::instruction* condition_in = ef.condition().get();
    const element::instruction* body_in = ef.body().get();
    const stack_allocation* initial_vr = state.allocator->get(initial_in);
    const stack_allocation* condition_vr = state.allocator->get(condition_in);
    const stack_allocation* body_vr = state.allocator->get(body_in);
    if (!initial_vr || !condition_vr || !body_vr)
        return ELEMENT_ERROR_UNKNOWN;

    // if we're looping, we have backbranches in the function so set that flag
    flags |= LMNT_DEFFLAG_HAS_BACKBRANCHES;

    std::vector<const stack_allocation*> condition_inputs, body_inputs;
    ELEMENT_OK_OR_RETURN(find_input_allocations(state, condition_in, ef.inputs, condition_inputs));
    ELEMENT_OK_OR_RETURN(find_input_allocations(state, body_in, ef.inputs, body_inputs));

    uint16_t initial_stack_idx, condition_stack_idx, body_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(initial_in, initial_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(condition_in, condition_stack_idx));
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(body_in, body_stack_idx));

    // compile the initial state
    ELEMENT_OK_OR_RETURN(compile_instruction(state, initial_in, output, flags));
    copy_stack_values(initial_stack_idx, body_stack_idx, initial_vr->count, output);
    // copy outputs to inputs before condition
    const size_t condition_index = output.size();
    for (uint16_t i = 0; i < condition_inputs.size(); ++i) {
        if (condition_inputs[i]) {
            const uint16_t input_stack_idx = state.calculate_stack_index(condition_inputs[i]->type(), condition_inputs[i]->index());
            copy_stack_values(body_stack_idx + i, input_stack_idx, 1, output);
        }
    }

    // compile condition logic
    ELEMENT_OK_OR_RETURN(compile_instruction(state, condition_vr->instruction, output, flags));
    output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, condition_stack_idx, 0, 0 });
    const size_t condition_branchcle_idx = output.size();
    output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCLE, 0, 0, 0 }); // target filled in at the end
    // copy outputs to inputs before body
    for (uint16_t i = 0; i < body_inputs.size(); ++i) {
        if (body_inputs[i]) {
            const uint16_t input_stack_idx = state.calculate_stack_index(body_inputs[i]->type(), body_inputs[i]->index());
            copy_stack_values(body_stack_idx + i, input_stack_idx, 1, output);
        }
    }

    // compile the loop body
    ELEMENT_OK_OR_RETURN(state.push_context(ef.body().get(), execution_type::conditional));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, body_vr->instruction, output, flags));
    ELEMENT_OK_OR_RETURN(state.pop_context());
    const size_t branch_past_idx = output.size();
    output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCH, 0, U16_LO(condition_index), U16_HI(condition_index) }); // branch --> condition
    const size_t past_idx = output.size();
    copy_stack_values(body_stack_idx, stack_idx, body_vr->count, output);

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

    return state.allocator->add(&ei, 1);
}

static element_result prepare_virtual_indexer(
    compiler_state& state,
    const element::instruction_indexer& ei)
{
    const element::instruction_for* ef = ei.for_instruction()->as<element::instruction_for>();
    if (!ef)
        return ELEMENT_ERROR_UNKNOWN;

    stack_allocation* ef_alloc = nullptr;
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, ef, &ef_alloc));
    state.use(&ei, ef);

    const element::instruction* ei_at = instruction_at(state, ef, ei.index, 1);
    if (!ei_at)
        return ELEMENT_ERROR_UNKNOWN;

    state.allocator->set_parent(&ei, ei_at, ei.index);
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
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    ELEMENT_OK_OR_RETURN(compile_instruction(state, ei.for_instruction().get(), output, flags));

    const element::instruction* ei_at = instruction_at(state, ei.for_instruction().get(), ei.index, 1);
    if (!ei_at)
        return ELEMENT_ERROR_UNKNOWN;

    uint16_t entry_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(ei_at, entry_stack_idx));

    copy_stack_values(entry_stack_idx, stack_idx, 1, output);
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

    ELEMENT_OK_OR_RETURN(state.add_constant(1));
    ELEMENT_OK_OR_RETURN(state.add_constant(element_value(es.options_count() - 1)));

    uint16_t max_count = 0;
    uint16_t index = 0;
    for (size_t i = 0; i < es.options_count(); ++i) {
        stack_allocation* option_vr;
        ELEMENT_OK_OR_RETURN(state.push_context(es.options_at(i).get(), execution_type::conditional));
        ELEMENT_OK_OR_RETURN(create_virtual_result(state, es.options_at(i).get(), &option_vr));
        ELEMENT_OK_OR_RETURN(state.pop_context());
        max_count = (std::max)(max_count, static_cast<uint16_t>(option_vr->count));
    }

    ELEMENT_OK_OR_RETURN(state.allocator->add(&es, max_count));
    ELEMENT_OK_OR_RETURN(state.allocator->add(&es, 1)); // selector scratch space
    return ELEMENT_OK;
}

static element_result prepare_virtual_select(
    compiler_state& state,
    const element::instruction_select& es)
{
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, es.selector().get()));
    state.use(&es, 1, es.selector().get(), 0);
    const size_t opts_size = es.options_count();
    for (size_t i = 0; i < opts_size; ++i) {
        ELEMENT_OK_OR_RETURN(state.push_context(es.options_at(i).get(), execution_type::conditional));
        ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, es.options_at(i).get()));
        ELEMENT_OK_OR_RETURN(state.pop_context());
        // TODO: handle failure (copy in?)
        state.allocator->set_parent(es.options_at(i).get(), 0, &es, 0, 0);
    }
    return ELEMENT_OK;
}

static element_result allocate_virtual_select(
    compiler_state& state,
    const element::instruction_select& es)
{
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, es.selector().get()));
    const size_t opts_size = es.options_count();
    for (size_t i = 0; i < opts_size; ++i) {
        ELEMENT_OK_OR_RETURN(state.push_context(es.options_at(i).get(), execution_type::conditional));
        ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, es.options_at(i).get()));
        ELEMENT_OK_OR_RETURN(state.pop_context());
    }
    return state.allocator->allocate(&es);
}

static element_result compile_select(
    compiler_state& state,
    const element::instruction_select& es,
    const uint16_t stack_idx,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    const size_t opts_size = es.options_count();
    uint16_t one_idx, last_valid_idx;
    ELEMENT_OK_OR_RETURN(state.find_constant(1, one_idx));
    ELEMENT_OK_OR_RETURN(state.find_constant(element_value(opts_size - 1), last_valid_idx));

    stack_allocation* selector_vr = state.allocator->get(es.selector().get());
    if (!selector_vr)
        return ELEMENT_ERROR_UNKNOWN;

    uint16_t selector_scratch_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(&es, selector_scratch_idx, 1));

    uint16_t selector_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(es.selector().get(), selector_stack_idx));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, es.selector().get(), output, flags));
    // clamp to the maximum valid index and truncate
    // we just check <= 0 for each condition so we don't care if it's already < 0
    // note we can't use the selector stack index here as it could be anything (a constant, an input...)
    // we also can't use the result spaces since there could already be calculated results in there
    output.emplace_back(lmnt_instruction{ LMNT_OP_MINSS, selector_stack_idx, last_valid_idx, selector_scratch_idx });
    output.emplace_back(lmnt_instruction{ LMNT_OP_TRUNCS, selector_scratch_idx, 0, selector_scratch_idx });

    const size_t branches_start_idx = output.size();
    for (size_t i = 0; i < opts_size; ++i) {
        output.emplace_back(lmnt_instruction{ LMNT_OP_CMPZ, selector_scratch_idx, 0, 0 });
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCHCLE, 0, 0, 0 }); // target filled in later
        output.emplace_back(lmnt_instruction{ LMNT_OP_SUBSS, selector_scratch_idx, one_idx, selector_scratch_idx });
    }

    uint16_t option0_stack_idx;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(es.options_at(0).get(), option0_stack_idx));
    const bool stacked = (option0_stack_idx == stack_idx);

    std::vector<size_t> option_branch_indexes(opts_size);

    for (size_t i = 0; i < opts_size; ++i) {
        stack_allocation* option_vr = state.allocator->get(es.options_at(i).get());
        if (!option_vr)
            return ELEMENT_ERROR_UNKNOWN;

        uint16_t option_stack_idx;
        ELEMENT_OK_OR_RETURN(state.calculate_stack_index(es.options_at(i).get(), option_stack_idx));

        const size_t option_idx = output.size();
        // fill in the target index for this branch option
        output[branches_start_idx + i * 3 + 1].arg2 = U16_LO(option_idx);
        output[branches_start_idx + i * 3 + 1].arg3 = U16_HI(option_idx);

        ELEMENT_OK_OR_RETURN(state.push_context(es.options_at(i).get(), execution_type::conditional));
        ELEMENT_OK_OR_RETURN(compile_instruction(state, es.options_at(i).get(), output, flags));
        ELEMENT_OK_OR_RETURN(state.pop_context());

        copy_stack_values(option_stack_idx, stack_idx, option_vr->count, output);
        option_branch_indexes[i] = output.size();
        output.emplace_back(lmnt_instruction{ LMNT_OP_BRANCH, 0, 0, 0 }); // target filled in later
    }

    const size_t past_idx = output.size();
    for (size_t i = 0; i < opts_size; ++i) {
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
    if (existing_vr) {
        if (vr)
            *vr = existing_vr;
        return ELEMENT_OK;
    }

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

    if (oresult == ELEMENT_OK) {
        if (vr)
            *vr = state.allocator->get(expr, 0);

        for (size_t i = 0; i < state.allocator->count(expr); ++i) {
            stack_allocation* a = state.allocator->get(expr, i);
            a->stage = compilation_stage::created;
            a->set_instruction = state.cur_instruction_index;
            a->last_used_instruction = state.cur_instruction_index;
        }
    }

    ++state.cur_instruction_index;
    return oresult;
}

static element_result prepare_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** vr)
{
    // whatever happens, note that this subtree is used in this context
    state.use_in_context(expr);

    // CSE: if this is non-null, we already did this
    stack_allocation* existing_vr = state.allocator->get(expr);
    if (!existing_vr)
        return ELEMENT_ERROR_UNKNOWN;

    // TODO: is it necessary to re-prepare something previously conditional?
    if (existing_vr->stage >= compilation_stage::prepared && existing_vr->executed_in == execution_type::unconditional) {
        if (vr)
            *vr = existing_vr;
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

    if (oresult == ELEMENT_OK) {
        if (vr)
            *vr = state.allocator->get(expr, 0);
        state.allocator->set_stage(expr, compilation_stage::prepared);
    }

    return oresult;
}

static element_result allocate_virtual_result(
    compiler_state& state,
    const element::instruction* expr,
    stack_allocation** vr)
{
    stack_allocation* existing_vr = state.allocator->get(expr);
    if (!existing_vr)
        return ELEMENT_ERROR_UNKNOWN;

    if (existing_vr->stage >= compilation_stage::allocated) {
        if (vr)
            *vr = existing_vr;
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

    if (oresult == ELEMENT_OK) {
        if (vr)
            *vr = state.allocator->get(expr, 0);
        state.allocator->set_stage(expr, compilation_stage::allocated);
    }

    return oresult;
}

static element_result compile_instruction(
    compiler_state& state,
    const element::instruction* expr,
    std::vector<lmnt_instruction>& output,
    lmnt_def_flags& flags)
{
    stack_allocation* vr = state.allocator->get(expr);
    if (!vr)
        return ELEMENT_ERROR_UNKNOWN;

    // only skip re-emitting bytecode if we've definitely already executed it
    if (vr->stage >= compilation_stage::compiled
        && (vr->executed_in == execution_type::unconditional || state.current_context->compiled_instructions.count(expr))) {
        return ELEMENT_OK;
    }

    uint16_t index;
    ELEMENT_OK_OR_RETURN(state.calculate_stack_index(expr, index));

    element_result oresult = ELEMENT_ERROR_NO_IMPL;

    if (const auto* ec = expr->as<element::instruction_constant>())
        oresult = compile_constant(state, *ec, index, output, flags);

    if (const auto* ei = expr->as<element::instruction_input>())
        oresult = compile_input(state, *ei, index, output, flags);

    if (const auto* es = expr->as<element::instruction_serialised_structure>())
        oresult = compile_serialised_structure(state, *es, index, output, flags);

    if (const auto* en = expr->as<element::instruction_nullary>())
        oresult = compile_nullary(state, *en, index, output, flags);

    if (const auto* eu = expr->as<element::instruction_unary>())
        oresult = compile_unary(state, *eu, index, output, flags);

    if (const auto* eb = expr->as<element::instruction_binary>())
        oresult = compile_binary(state, *eb, index, output, flags);

    if (const auto* ei = expr->as<element::instruction_if>())
        oresult = compile_if(state, *ei, index, output, flags);

    if (const auto* ef = expr->as<element::instruction_for>())
        oresult = compile_for(state, *ef, index, output, flags);

    if (const auto* ei = expr->as<element::instruction_indexer>())
        oresult = compile_indexer(state, *ei, index, output, flags);

    if (const auto* sel = expr->as<element::instruction_select>())
        oresult = compile_select(state, *sel, index, output, flags);

    if (oresult == ELEMENT_OK) {
        state.allocator->set_stage(expr, compilation_stage::compiled);
        state.current_context->compiled_instructions.emplace(expr);
    }

    return oresult;
}

element_result element_lmnt_find_constants(
    const element_lmnt_compiler_ctx& ctx,
    const element::instruction_const_shared_ptr& expr,
    std::unordered_map<element_value, size_t>& candidates)
{
    if (const auto* ec = expr->as<element::instruction_constant>()) {
        const element_value value = ec->value();
        auto it = candidates.find(value);
        if (it != candidates.end())
            ++it->second;
        else
            candidates.emplace(value, 1);
    }
    for (const auto& dep : expr->dependents()) {
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
    compiler_state state{ ctx, instruction.get(), constants, static_cast<uint16_t>(inputs_count) };
    // TODO: check for single constant fast path
    stack_allocation* vr = nullptr;
    ELEMENT_OK_OR_RETURN(create_virtual_result(state, instruction.get(), &vr));
    // set ourselves as being an output
    ELEMENT_OK_OR_RETURN(state.allocator->set_pinned(instruction.get(), allocation_type::output, 0, vr->count));
    // continue with compilation
    ELEMENT_OK_OR_RETURN(prepare_virtual_result(state, instruction.get()));
    ELEMENT_OK_OR_RETURN(allocate_virtual_result(state, instruction.get()));
    ELEMENT_OK_OR_RETURN(compile_instruction(state, instruction.get(), output.instructions, output.flags));
    output.inputs_count = inputs_count;

    output.outputs_count = vr->count;
    output.local_stack_count = state.allocator->get_max_stack_usage();
    return ELEMENT_OK;
}