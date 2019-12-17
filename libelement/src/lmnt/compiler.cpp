#include "lmnt/compiler.hpp"
#include "lmnt/common.h"
#include "interpreter_internal.hpp"
#include <cstdlib>
#include <cassert>
#include <unordered_map>

static std::unordered_map<element_unary_op, lmnt_opcode> unary_scalar_ops = {
    {element_unary_op::abs, LMNT_OP_ABSS},
    {element_unary_op::acos, LMNT_OP_ACOS},
    {element_unary_op::asin, LMNT_OP_ASIN},
    {element_unary_op::atan, LMNT_OP_ATAN},
    {element_unary_op::ceil, LMNT_OP_CEILS},
    {element_unary_op::cos, LMNT_OP_COS},
    {element_unary_op::floor, LMNT_OP_FLOORS},
    // {element_unary_op::ln, LMNT_OP_LN},
    // {element_unary_op::round, LMNT_OP_ROUNDS},
    {element_unary_op::sin, LMNT_OP_SIN},
    {element_unary_op::tan, LMNT_OP_TAN},
};

static std::unordered_map<element_unary_op, lmnt_opcode> unary_vector_ops = {
    {element_unary_op::abs, LMNT_OP_ABSV},
    {element_unary_op::ceil, LMNT_OP_CEILV},
    {element_unary_op::floor, LMNT_OP_FLOORV},
    // {element_unary_op::round, LMNT_OP_ROUNDV},
};

static std::unordered_map<element_binary_op, lmnt_opcode> binary_scalar_ops = {
    {element_binary_op::add, LMNT_OP_ADDSS},
    // {element_binary_op::atan2, LMNT_OP_ATAN2},
    {element_binary_op::div, LMNT_OP_DIVSS},
    // {element_binary_op::log, LMNT_OP_LOG},
    {element_binary_op::max, LMNT_OP_MAXSS},
    {element_binary_op::min, LMNT_OP_MINSS},
    {element_binary_op::mul, LMNT_OP_MULSS},
    {element_binary_op::pow, LMNT_OP_POWSS},
    {element_binary_op::rem, LMNT_OP_MODSS}, // TODO
    {element_binary_op::sub, LMNT_OP_SUBSS},
};

// TODO: VS variants...
static std::unordered_map<element_binary_op, lmnt_opcode> binary_vector_ops = {
    {element_binary_op::add, LMNT_OP_ADDVV},
    {element_binary_op::div, LMNT_OP_DIVVV},
    {element_binary_op::max, LMNT_OP_MAXVV},
    {element_binary_op::min, LMNT_OP_MINVV},
    {element_binary_op::mul, LMNT_OP_MULVV},
    {element_binary_op::pow, LMNT_OP_POWVV}, // TODO: special-case sqrt
    {element_binary_op::rem, LMNT_OP_MODVV}, // TODO
    {element_binary_op::sub, LMNT_OP_SUBVV},
};

struct compiler_ctx
{
    element_interpreter_ctx& ictx;
    element_lmnt_compile_options opts;
    element_lmnt_archive_ctx& archive;
    element_lmnt_compiled_function* fn;
    std::shared_ptr<std::vector<element_value>> constants;

    std::unordered_map<const element_expression*, element_lmnt_stack_entry>& entries() { return archive.entries[fn]; }
};

element_result buf_raw_write_and_update(char* b, size_t bsize, size_t& i, const void* d, size_t dsize)
{
    if (i + dsize > bsize) return ELEMENT_ERROR_INVALID_OPERATION;
    std::memcpy(b + i, d, dsize);
    i += dsize;
    return ELEMENT_OK;
}

template <typename T>
element_result buf_write_and_update(char* b, size_t bsize, size_t& i, const T& t)
{
    return buf_raw_write_and_update(b, bsize, i, &t, sizeof(T));
}

static element_result generate_lmnt_constants(
    compiler_ctx& ctx,
    const expression_const_shared_ptr& expr)
{
    if (auto ce = expr->as<element_constant>()) {
        size_t cidx = ctx.archive.get_constant(ce->value());
        ctx.entries()[expr.get()] = { element_lmnt_stack_entry::entry_type::constant, cidx, 1 };
    } else {
        for (const auto& t : expr->dependents()) {
            ELEMENT_OK_OR_RETURN(generate_lmnt_constants(ctx, t));
        }
    }
    return ELEMENT_OK;
}

static element_result generate_lmnt_ops(
    compiler_ctx& ctx,
    const expression_const_shared_ptr& expr,
    size_t depth = 0)
{
    // TODO: smarts of any sort - especially vectorisation
    if (auto ce = expr->as<element_constant>()) {
        // already computed by previous pass
        // TODO: if constant -> output, do a COPY
        return ELEMENT_OK;
    } else if (auto ie = expr->as<element_input>()) {
        assert(ie->index() + ie->get_size() <= ctx.fn->inputs_size);
        ctx.entries()[expr.get()] = {element_lmnt_stack_entry::entry_type::input, ie->index(), ie->get_size()};
        // TODO: if input -> output, do a COPY
        return ELEMENT_OK;
    } else if (auto se = expr->as<element_structure>()) {
        assert(depth == 0);
        for (const auto& d : se->dependents()) {
            // do *not* increment depth here: let the dependents write to the outputs as needed
            ELEMENT_OK_OR_RETURN(generate_lmnt_ops(ctx, d, depth));
        }
        return ELEMENT_OK;
    } else if (auto ue = expr->as<element_unary>()) {
        ELEMENT_OK_OR_RETURN(generate_lmnt_ops(ctx, ue->input(), depth + 1));
        element_lmnt_instruction op;
        // find our opcode
        auto itu = unary_scalar_ops.find(ue->operation());
        if (itu != unary_scalar_ops.end())
            op.opcode = itu->second;
        else
            return ELEMENT_ERROR_NO_IMPL;
        // find our input on the stack
        auto it1 = ctx.entries().find(ue->input().get());
        if (it1 != ctx.entries().end())
            op.arg1 = it1->second;
        else
            return ELEMENT_ERROR_INVALID_OPERATION;
        if (depth > 0)
            op.arg3 = { element_lmnt_stack_entry::entry_type::local, ctx.fn->locals_size++, 1 };
        else
            op.arg3 = { element_lmnt_stack_entry::entry_type::output, ctx.fn->outputs_matched++, 1 };
        ctx.fn->ops.push_back(op);
        ctx.entries()[expr.get()] = op.arg3;
        return ELEMENT_OK;
    } else if (auto be = expr->as<element_binary>()) {
        ELEMENT_OK_OR_RETURN(generate_lmnt_ops(ctx, be->input1(), depth + 1));
        ELEMENT_OK_OR_RETURN(generate_lmnt_ops(ctx, be->input2(), depth + 1));
        element_lmnt_instruction op;
        // find our opcode
        auto itb = binary_scalar_ops.find(be->operation());
        if (itb != binary_scalar_ops.end())
            op.opcode = itb->second;
        else
            return ELEMENT_ERROR_NO_IMPL;
        // find our inputs on the stack
        auto it1 = ctx.entries().find(be->input1().get());
        if (it1 != ctx.entries().end())
            op.arg1 = it1->second;
        else
            return ELEMENT_ERROR_INVALID_OPERATION;
        auto it2 = ctx.entries().find(be->input2().get());
        if (it2 != ctx.entries().end())
            op.arg2 = it2->second;
        else
            return ELEMENT_ERROR_INVALID_OPERATION;
        if (depth > 0)
            op.arg3 = { element_lmnt_stack_entry::entry_type::local, ctx.fn->locals_size++, 1 };
        else
            op.arg3 = { element_lmnt_stack_entry::entry_type::output, ctx.fn->outputs_matched++, 1 };
        ctx.fn->ops.push_back(op);
        ctx.entries()[expr.get()] = op.arg3;
        return ELEMENT_OK;
    } else {
        assert(false);
        return ELEMENT_ERROR_NO_IMPL;
    }
}


static size_t get_input_size(const expression_const_shared_ptr& expr)
{
    auto ie = expr->as<element_input>();
    if (ie) {
        return ie->get_size();
    } else {
        return std::accumulate(expr->dependents().begin(), expr->dependents().end(), size_t(0),
            [](size_t a, const auto& b) { return a + get_input_size(b); });
    }
}


static element_result lmnt_compile(
    element_interpreter_ctx& ictx,
    const element_compiled_function& cfn,
    const element_lmnt_compile_options& opts,
    element_lmnt_archive_ctx& archive)
{
    // generate LMNT function from etree
    compiler_ctx result { ictx, opts, archive };
    result.archive.functions.emplace_back();
    result.fn = &result.archive.functions.back();
    result.fn->function = cfn.function;
    ELEMENT_OK_OR_RETURN(generate_lmnt_constants(result, cfn.expression));

    // don't rely on function's inputs/outputs: no guarantee they are concrete
    result.fn->inputs_size = get_input_size(cfn.expression);
    result.fn->outputs_size = cfn.expression->get_size();

    ELEMENT_OK_OR_RETURN(generate_lmnt_ops(result, cfn.expression));

    return ELEMENT_OK;
}


static element_result build_archive(
    const element_lmnt_archive_ctx& archive,
    char* buffer, size_t& size)
{
    const size_t defs_size = sizeof(lmnt_def) * archive.functions.size();  // assumes no bases, which is a feature we probably want to kill anyway
    const size_t constants_size = sizeof(lmnt_value) * archive.constants.size();

    size_t idx = 0;

    // determine indexes for strings and code
    lmnt_offset strings_size = 0;
    lmnt_loffset code_size = 0;
    std::vector<lmnt_offset> string_indexes;
    string_indexes.reserve(archive.functions.size());
    std::vector<lmnt_loffset> code_indexes;
    code_indexes.reserve(archive.functions.size());
    for (const auto& fn : archive.functions) {
        string_indexes.push_back(strings_size);
        strings_size += lmnt_offset(fn.function->name().length() + sizeof('\0') + sizeof(lmnt_offset));
        code_indexes.push_back(code_size);
        code_size += lmnt_loffset(fn.ops.size() * sizeof(lmnt_instruction) + sizeof(lmnt_loffset));
    }

    // calculate rounding before constants
    const size_t PAD_TO = 8;
    const size_t rounding = (PAD_TO - ((sizeof(lmnt_archive_header) + strings_size + defs_size + code_size) % PAD_TO)) % PAD_TO;

    const size_t archive_size = sizeof(lmnt_archive_header) + strings_size + defs_size + code_size + rounding + constants_size;

    // if we're just being asked for the size, leave now
    if (!buffer) {
        size = archive_size;
        return ELEMENT_OK;
    }

    if (archive_size > size) return ELEMENT_ERROR_INVALID_SIZE;
    size = archive_size;

    // write header
    lmnt_archive_header hdr;
    std::memset(&hdr, 0, sizeof(lmnt_archive_header));
    hdr.magic[0] = 'L';
    hdr.magic[1] = 'M';
    hdr.magic[2] = 'N';
    hdr.magic[3] = 'T';
    hdr.version_major = 0;
    hdr.version_minor = 0;
    hdr.strings_length = lmnt_loffset(strings_size);
    hdr.defs_length = lmnt_loffset(defs_size);
    hdr.code_length = lmnt_loffset(code_size) + lmnt_loffset(rounding);
    hdr.constants_length = lmnt_loffset(constants_size);
    ELEMENT_OK_OR_RETURN(buf_write_and_update(buffer, size, idx, hdr));

    // write strings
    for (const auto& fn : archive.functions) {
        std::string name = fn.function->name();
        ELEMENT_OK_OR_RETURN(buf_write_and_update(buffer, size, idx, uint16_t(name.length() + 1)));
        ELEMENT_OK_OR_RETURN(buf_raw_write_and_update(buffer, size, idx, name.c_str(), name.length() + 1));
    }

    // write defs
    for (size_t i = 0; i < archive.functions.size(); ++i) {
        const element_lmnt_compiled_function& fn = archive.functions[i];
        lmnt_def def;
        def.length = sizeof(lmnt_def);
        def.name = string_indexes[i];
        def.flags = 0;  // TODO: single-constant optimisation
        def.code = code_indexes[i]; // TODO: single-constant optimisation
        def.stack_count_unaligned = lmnt_offset(fn.locals_size + fn.inputs_size + fn.outputs_size);
        def.stack_count_aligned = lmnt_offset(fn.locals_size + fn.inputs_size + fn.outputs_size);
        def.base_args_count = 0;
        def.args_count = lmnt_offset(fn.inputs_size);
        def.rvals_count = lmnt_offset(fn.outputs_size);
        def.bases_count = 0;
        ELEMENT_OK_OR_RETURN(buf_write_and_update(buffer, size, idx, def));
    }

    auto get_arg_index = [&archive](const element_lmnt_compiled_function& fn, const element_lmnt_stack_entry& arg)
    {
        lmnt_offset argi = lmnt_offset(arg.index);
        switch (arg.type) {
        case element_lmnt_stack_entry::entry_type::local:  argi += lmnt_offset(fn.outputs_size); // fallthrough
        case element_lmnt_stack_entry::entry_type::output: argi += lmnt_offset(fn.inputs_size);  // fallthrough
        case element_lmnt_stack_entry::entry_type::input:  argi += lmnt_offset(archive.constants.size()); // fallthrough
        case element_lmnt_stack_entry::entry_type::constant: default: break;
        }
        return argi;
    };

    // write code
    for (const auto& fn : archive.functions) {
        ELEMENT_OK_OR_RETURN(buf_write_and_update(buffer, size, idx, lmnt_loffset(fn.ops.size())));
        for (const auto& op : fn.ops) {
            lmnt_instruction iop;
            iop.opcode = op.opcode;
            iop.arg1 = get_arg_index(fn, op.arg1);
            iop.arg2 = get_arg_index(fn, op.arg2);
            iop.arg3 = get_arg_index(fn, op.arg3);
            ELEMENT_OK_OR_RETURN(buf_write_and_update(buffer, size, idx, iop));
        }
    }

    // write padding
    char padding[PAD_TO] = {0};
    if (rounding)
        ELEMENT_OK_OR_RETURN(buf_raw_write_and_update(buffer, size, idx, padding, rounding));

    // write constants
    ELEMENT_OK_OR_RETURN(buf_raw_write_and_update(buffer, size, idx, archive.constants.data(), archive.constants.size() * sizeof(lmnt_value)));

    return ELEMENT_OK;
}


element_result element_lmnt_archive_init(element_lmnt_archive_ctx** ctx)
{
    *ctx = new element_lmnt_archive_ctx;
    return ELEMENT_OK;
}

void element_lmnt_archive_delete(element_lmnt_archive_ctx* ctx)
{
    delete ctx;
}

element_result element_lmnt_compile(
    element_interpreter_ctx* ctx,
    const element_compiled_function* cfn,
    const element_lmnt_compile_options* opts,
    element_lmnt_archive_ctx* archive)
{
    assert(ctx);
    assert(cfn);
    assert(opts);
    assert(archive);
    return lmnt_compile(*ctx, *cfn, *opts, *archive);
}

element_result element_lmnt_archive_build(
    const element_lmnt_archive_ctx* archive,
    char* buffer, size_t* size)
{
    assert(archive);
    assert(size);
    return build_archive(*archive, buffer, *size);
}
