#ifndef LMNT_DISPATCH_JUMPTABLE_H
#define LMNT_DISPATCH_JUMPTABLE_H

#include <assert.h>

#include "lmnt/config.h"
#include "lmnt/common.h"
#include "lmnt/interpreter.h"

#include "ops_bounds_impl.h"
#include "ops_branch_impl.h"
#include "ops_fncall_impl.h"
#include "ops_math_impl.h"
#include "ops_misc_impl.h"
#include "ops_trig_impl.h"

static const char* const dispatch_method(void)
{
    return "jump table";
}

LMNT_ATTR_FAST static const lmnt_op_fn lmnt_op_functions[LMNT_OP_END] = {
    lmnt_op_return,      // 0x00 
    lmnt_op_noop,        // 0x01
    lmnt_op_assignss,    // 0x02
    lmnt_op_assignvv,    // 0x03
    lmnt_op_noop,        // 0x04
    lmnt_op_assignsv,    // 0x05
    lmnt_op_noop,        // 0x06
    lmnt_op_noop,        // 0x07
    lmnt_op_assignibs,   // 0x08
    lmnt_op_assignibv,   // 0x09
//    lmnt_op_dloadiis,
//    lmnt_op_dloadiiv,
//    lmnt_op_dloadirs,
//    lmnt_op_dloadirv,
//    lmnt_op_dseclen,
    lmnt_op_noop,        // 0x0A
    lmnt_op_noop,        // 0x0B
    lmnt_op_addss,       // 0x0C
//    lmnt_op_addvv,
    lmnt_op_subss,       // 0x0D
//    lmnt_op_subvv,
    lmnt_op_mulss,       // 0x0E
//    lmnt_op_mulvv,
    lmnt_op_divss,       // 0x0F
//    lmnt_op_divvv,
    lmnt_op_noop,        // 0x10
    lmnt_op_noop,        // 0x11
    lmnt_op_noop,        // 0x12
    lmnt_op_remss,       // 0x13
//    lmnt_op_remvv,
    lmnt_op_noop,        // 0x14
    lmnt_op_noop,        // 0x15
    lmnt_op_noop,        // 0x16
    lmnt_op_noop,        // 0x17
    lmnt_op_noop,        // 0x18
    lmnt_op_noop,        // 0x19
    lmnt_op_noop,        // 0x1A
    lmnt_op_noop,        // 0x1B
    lmnt_op_noop,        // 0x1C
    lmnt_op_sinr,        // 0x1D
    lmnt_op_cosr,        // 0x1E
    lmnt_op_tanr,        // 0x1F
    lmnt_op_noop,        // 0x20
    lmnt_op_noop,        // 0x21
    lmnt_op_noop,        // 0x22
    lmnt_op_noop,        // 0x23
    lmnt_op_asinr,       // 0x24
    lmnt_op_noop,        // 0x25
    lmnt_op_noop,        // 0x26
    lmnt_op_noop,        // 0x27
    lmnt_op_noop,        // 0x28
    lmnt_op_noop,        // 0x29
    lmnt_op_noop,        // 0x2A
    lmnt_op_noop,        // 0x2B
    lmnt_op_acosr,       // 0x2C
    lmnt_op_noop,        // 0x2D
    lmnt_op_noop,        // 0x2E
    lmnt_op_noop,        // 0x2F
    lmnt_op_noop,        // 0x30
    lmnt_op_noop,        // 0x31
    lmnt_op_noop,        // 0x32
    lmnt_op_noop,        // 0x33
    lmnt_op_atanr,       // 0x34
    lmnt_op_noop,        // 0x35
    lmnt_op_atan2r,      // 0x36
    lmnt_op_sincosr,     // 0x37
    lmnt_op_powss,       // 0x38
//    lmnt_op_powvv,
//    lmnt_op_powvs,
    lmnt_op_noop,        // 0x39
    lmnt_op_noop,        // 0x3A
    lmnt_op_noop,        // 0x3B
    lmnt_op_noop,        // 0x3C
    lmnt_op_noop,        // 0x3D
    lmnt_op_sqrts,       // 0x3E
//    lmnt_op_sqrtv,
//    lmnt_op_ln,
    lmnt_op_log2,        // 0x40
//    lmnt_op_log10,
    lmnt_op_noop,        // 0x41
    lmnt_op_abss,        // 0x42
//    lmnt_op_absv,
//    lmnt_op_sumv,
    lmnt_op_minss,       // 0x43
//    lmnt_op_minvv,
    lmnt_op_noop,        // 0x44
    lmnt_op_noop,        // 0x45
    lmnt_op_maxss,       // 0x46
//    lmnt_op_maxvv,
//    lmnt_op_minvs,
//    lmnt_op_maxvs,
    lmnt_op_noop,        // 0x47
    lmnt_op_noop,        // 0x48
    lmnt_op_floors,      // 0x49
//    lmnt_op_floorv,
    lmnt_op_rounds,      // 0x4A
//    lmnt_op_roundv,
    lmnt_op_ceils,       // 0x4B
//    lmnt_op_ceilv,
//    lmnt_op_truncs,
//    lmnt_op_truncv,
    lmnt_op_indexris,    // 0x4C
    lmnt_op_noop,        // 0x4D
    lmnt_op_indexrir,    // 0x4E
    lmnt_op_noop,        // 0x4F
    lmnt_op_branch,      // 0x50
    lmnt_op_branchz,     // 0x51
    lmnt_op_noop,        // 0x52
    lmnt_op_branchnz,    // 0x53
    lmnt_op_noop,        // 0x54
    lmnt_op_branchpos,   // 0x55
    lmnt_op_noop,        // 0x56
    lmnt_op_branchneg,   // 0x57
    lmnt_op_noop,        // 0x58
    lmnt_op_branchun,    // 0x59
    lmnt_op_noop,        // 0x5A
    lmnt_op_cmp,         // 0x5B
    lmnt_op_cmpz,        // 0x5C
    lmnt_op_branchceq,   // 0x5D
    lmnt_op_branchcne,   // 0x5E
    lmnt_op_branchclt,   // 0x5F
    lmnt_op_branchcle,   // 0x60
    lmnt_op_branchcgt,   // 0x61
    lmnt_op_branchcge,   // 0x62
    lmnt_op_branchcun,   // 0x63
    lmnt_op_assignceq,   // 0x64
    lmnt_op_assigncne,   // 0x65
    lmnt_op_assignclt,   // 0x66
    lmnt_op_assigncle,   // 0x67
    lmnt_op_assigncgt,   // 0x68
    lmnt_op_assigncge,   // 0x69
    lmnt_op_assigncun,   // 0x6A
    lmnt_op_sumv,        // 0x6B
//    lmnt_op_extcall,
};

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result execute_instruction(lmnt_ictx* ctx, const lmnt_instruction op)
{
    assert(op.opcode < LMNT_OP_END);
    return lmnt_op_functions[op.opcode](ctx, op.arg1, op.arg2, op.arg3);
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result execute_function(lmnt_ictx* ctx, const lmnt_code* defcode, const lmnt_instruction* instructions)
{
    lmnt_result opresult = LMNT_OK;
    // Grab the current instruction as a local rather than updating the ctx every time - faster
    lmnt_loffset instr;
    const lmnt_loffset icount = defcode->instructions_count;
    for (instr = ctx->cur_instr; instr < icount; ++instr)
    {
        opresult = execute_instruction(ctx, instructions[instr]);
#if defined(LMNT_DEBUG_PRINT_EVALUATED_INSTRUCTIONS)
        print_execution_context(ctx, instr, instructions[instr]);
#endif
        if (LMNT_UNLIKELY(opresult | (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))) {
            if (opresult == LMNT_BRANCHING) {
                // the context's instruction pointer has been updated, refresh
                instr = ctx->cur_instr - 1; // will be incremented by loop
                continue;
            } else {
                break;
            }
        }
    }
    ctx->cur_instr = instr;
    return opresult;
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result interrupt_function(lmnt_ictx* ctx)
{
    ctx->status_flags |= LMNT_ISTATUS_INTERRUPTED;
    return LMNT_OK;
}

#endif
