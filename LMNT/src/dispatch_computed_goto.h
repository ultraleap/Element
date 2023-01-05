#ifndef LMNT_DISPATCH_COMPUTED_GOTO_H
#define LMNT_DISPATCH_COMPUTED_GOTO_H

#include "lmnt/config.h"
#include "lmnt/common.h"
#include "lmnt/interpreter.h"

#define LMNT_INLINE_OP LMNT_FORCEINLINE
#include "ops_bounds_impl.h"
#include "ops_branch_impl.h"
#include "ops_fncall_impl.h"
#include "ops_math_impl.h"
#include "ops_misc_impl.h"
#include "ops_trig_impl.h"

static const char* const dispatch_method(void)
{
    return "computed gotos";
}

// cannot be inlined due to static table
LMNT_ATTR_FAST static lmnt_result execute_function(lmnt_ictx* ctx, const lmnt_code* defcode, const lmnt_instruction* instructions)
{
    // set up table
    static void* const jump_targets[LMNT_OP_END] = {
        &&op_return,      // 0x00 
        &&op_noop,        // 0x01
        &&op_assignss,    // 0x02
        &&op_assignvv,    // 0x03
        &&op_noop,        // 0x04
        &&op_assignsv,    // 0x05
        &&op_noop,        // 0x06
        &&op_noop,        // 0x07
        &&op_assignibs,   // 0x08
        &&op_assignibv,   // 0x09
//        &&op_dloadiis,
//        &&op_dloadiiv,
//        &&op_dloadirs,
//        &&op_dloadirv,
//        &&op_dseclen,
        &&op_noop,        // 0x0A
        &&op_noop,        // 0x0B
        &&op_addss,       // 0x0C
//        &&op_addvv,
        &&op_subss,       // 0x0D
//        &&op_subvv,
        &&op_mulss,       // 0x0E
//        &&op_mulvv,
        &&op_divss,       // 0x0F
//        &&op_divvv,
        &&op_noop,        // 0x10
        &&op_noop,        // 0x11
        &&op_noop,        // 0x12
        &&op_remss,       // 0x13
//        &&op_remvv,
        &&op_noop,        // 0x14
        &&op_noop,        // 0x15
        &&op_noop,        // 0x16
        &&op_noop,        // 0x17
        &&op_noop,        // 0x18
        &&op_noop,        // 0x19
        &&op_noop,        // 0x1A
        &&op_noop,        // 0x1B
        &&op_noop,        // 0x1C
        &&op_sinr,        // 0x1D
        &&op_cosr,        // 0x1E
        &&op_tanr,        // 0x1F
        &&op_noop,        // 0x20
        &&op_noop,        // 0x21
        &&op_noop,        // 0x22
        &&op_noop,        // 0x23
        &&op_asinr,       // 0x24
        &&op_noop,        // 0x25
        &&op_noop,        // 0x26
        &&op_noop,        // 0x27
        &&op_noop,        // 0x28
        &&op_noop,        // 0x29
        &&op_noop,        // 0x2A
        &&op_noop,        // 0x2B
        &&op_acosr,       // 0x2C
        &&op_noop,        // 0x2D
        &&op_noop,        // 0x2E
        &&op_noop,        // 0x2F
        &&op_noop,        // 0x30
        &&op_noop,        // 0x31
        &&op_noop,        // 0x32
        &&op_noop,        // 0x33
        &&op_atanr,       // 0x34
        &&op_noop,        // 0x35
        &&op_atan2r,      // 0x36
        &&op_sincosr,     // 0x37
        &&op_powss,       // 0x38
//        &&op_powvv,
//        &&op_powvs,
        &&op_noop,        // 0x39
        &&op_noop,        // 0x3A
        &&op_noop,        // 0x3B
        &&op_noop,        // 0x3C
        &&op_noop,        // 0x3D
        &&op_sqrts,       // 0x3E
//        &&op_sqrtv,
//        &&op_ln,
        &&op_log2,        // 0x40
//        &&op_log10,
        &&op_noop,        // 0x41
        &&op_abss,        // 0x42
//        &&op_absv,
//        &&op_sumv,
        &&op_minss,       // 0x43
//        &&op_minvv,
        &&op_noop,        // 0x44
        &&op_noop,        // 0x45
        &&op_maxss,       // 0x46
//        &&op_maxvv,
//        &&op_minvs,
//        &&op_maxvs,
        &&op_noop,        // 0x47
        &&op_noop,        // 0x48
        &&op_floors,      // 0x49
//        &&op_floorv,
        &&op_rounds,      // 0x4A
//        &&op_roundv,
        &&op_ceils,       // 0x4B
//        &&op_ceilv,
//        &&op_truncs,
//        &&op_truncv,
        &&op_indexris,    // 0x4C
        &&op_noop,        // 0x4D
        &&op_indexrir,    // 0x4E
        &&op_noop,        // 0x4F
        &&op_branch,      // 0x50
        &&op_branchz,     // 0x51
        &&op_noop,        // 0x52
        &&op_branchnz,    // 0x53
        &&op_noop,        // 0x54
        &&op_branchpos,   // 0x55
        &&op_noop,        // 0x56
        &&op_branchneg,   // 0x57
        &&op_noop,        // 0x58
        &&op_branchun,    // 0x59
        &&op_noop,        // 0x5A
        &&op_cmp,         // 0x5B
        &&op_cmpz,        // 0x5C
        &&op_branchceq,   // 0x5D
        &&op_branchcne,   // 0x5E
        &&op_branchclt,   // 0x5F
        &&op_branchcle,   // 0x60
        &&op_branchcgt,   // 0x61
        &&op_branchcge,   // 0x62
        &&op_branchcun,   // 0x63
        &&op_assignceq,   // 0x64
        &&op_assigncne,   // 0x65
        &&op_assignclt,   // 0x66
        &&op_assigncle,   // 0x67
        &&op_assigncgt,   // 0x68
        &&op_assigncge,   // 0x69
        &&op_assigncun,   // 0x6A
        &&op_sumv,        // 0x6B
//        &&op_extcall,
    };

    lmnt_result opresult = LMNT_OK;
    // Grab the current instruction as a local rather than updating the ctx every time - faster
    lmnt_loffset instr = ctx->cur_instr - 1; // incremented at start of dispatch
    const lmnt_loffset icount = defcode->instructions_count;
    lmnt_instruction op;

dispatch:
    if (LMNT_UNLIKELY(opresult | (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))) {
        if (opresult == LMNT_BRANCHING) {
            // the context's instruction pointer has been updated, refresh
            instr = ctx->cur_instr - 1; // will be incremented momentarily
        } else {
            goto endcheck;
        }
    }

#define GENERATE_DISPATCHOK() \
    if (++instr >= icount || (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))\
        goto endcheck;\
    op = instructions[instr];\
    goto *jump_targets[op.opcode];

    GENERATE_DISPATCHOK();

#define GENERATE_OP(name, nextstop) \
op_##name: \
    opresult = lmnt_op_##name(ctx, op.arg1, op.arg2, op.arg3); \
    goto nextstop;
#define GENERATE_OP_NOFAIL(name) \
op_##name: \
    lmnt_op_##name(ctx, op.arg1, op.arg2, op.arg3); \
    GENERATE_DISPATCHOK();

op_noop:
    GENERATE_DISPATCHOK();
op_return:
    opresult = LMNT_RETURNING;
    goto end;
GENERATE_OP_NOFAIL(assignss);
GENERATE_OP_NOFAIL(assignvv);
GENERATE_OP_NOFAIL(assignsv);
GENERATE_OP_NOFAIL(assignibs);
GENERATE_OP_NOFAIL(assignibv);
/*
GENERATE_OP(dloadiis, dispatch);
GENERATE_OP(dloadiiv, dispatch);
GENERATE_OP(dloadirs, dispatch);
GENERATE_OP(dloadirv, dispatch);
GENERATE_OP_NOFAIL(dseclen);
*/
GENERATE_OP_NOFAIL(addss);
//GENERATE_OP_NOFAIL(addvv);
GENERATE_OP_NOFAIL(subss);
//GENERATE_OP_NOFAIL(subvv);
GENERATE_OP_NOFAIL(mulss);
//GENERATE_OP_NOFAIL(mulvv);
GENERATE_OP_NOFAIL(divss);
//GENERATE_OP_NOFAIL(divvv);
GENERATE_OP_NOFAIL(remss);
//GENERATE_OP_NOFAIL(remvv);
GENERATE_OP_NOFAIL(sinr);
GENERATE_OP_NOFAIL(cosr);
GENERATE_OP_NOFAIL(tanr);
GENERATE_OP_NOFAIL(asinr);
GENERATE_OP_NOFAIL(acosr);
GENERATE_OP_NOFAIL(atanr);
GENERATE_OP_NOFAIL(atan2r);
GENERATE_OP_NOFAIL(sincosr);
GENERATE_OP_NOFAIL(powss);
//GENERATE_OP_NOFAIL(powvv);
//GENERATE_OP_NOFAIL(powvs);
GENERATE_OP_NOFAIL(sqrts);
//GENERATE_OP_NOFAIL(sqrtv);
//GENERATE_OP_NOFAIL(ln);
GENERATE_OP_NOFAIL(log2);
//GENERATE_OP_NOFAIL(log10);
GENERATE_OP_NOFAIL(abss);
//GENERATE_OP_NOFAIL(absv);
GENERATE_OP_NOFAIL(sumv);
GENERATE_OP_NOFAIL(minss);
//GENERATE_OP_NOFAIL(minvv);
GENERATE_OP_NOFAIL(maxss);
//GENERATE_OP_NOFAIL(maxvv);
//GENERATE_OP_NOFAIL(minvs);
//GENERATE_OP_NOFAIL(maxvs);
GENERATE_OP_NOFAIL(floors);
//GENERATE_OP_NOFAIL(floorv);
GENERATE_OP_NOFAIL(rounds);
//GENERATE_OP_NOFAIL(roundv);
GENERATE_OP_NOFAIL(ceils);
//GENERATE_OP_NOFAIL(ceilv);
//GENERATE_OP_NOFAIL(truncs);
//GENERATE_OP_NOFAIL(truncv);
GENERATE_OP(indexris, dispatch);
GENERATE_OP(indexrir, dispatch);
GENERATE_OP(branch, branchcheck);
GENERATE_OP(branchz, branchcheck);
GENERATE_OP(branchnz, branchcheck);
GENERATE_OP(branchpos, branchcheck);
GENERATE_OP(branchneg, branchcheck);
GENERATE_OP(branchun, branchcheck);
GENERATE_OP_NOFAIL(cmp);
GENERATE_OP_NOFAIL(cmpz);
GENERATE_OP(branchceq, branchcheck);
GENERATE_OP(branchcne, branchcheck);
GENERATE_OP(branchclt, branchcheck);
GENERATE_OP(branchcle, branchcheck);
GENERATE_OP(branchcgt, branchcheck);
GENERATE_OP(branchcge, branchcheck);
GENERATE_OP(branchcun, branchcheck);
GENERATE_OP_NOFAIL(assignceq);
GENERATE_OP_NOFAIL(assigncne);
GENERATE_OP_NOFAIL(assignclt);
GENERATE_OP_NOFAIL(assigncle);
GENERATE_OP_NOFAIL(assigncgt);
GENERATE_OP_NOFAIL(assigncge);
GENERATE_OP_NOFAIL(assigncun);
//GENERATE_OP(extcall, dispatch);

#undef GENERATE_DISPATCHOK
#undef GENERATE_OP_NOFAIL
#undef GENERATE_OP

endcheck:
    if (opresult == LMNT_OK && (ctx->status_flags & LMNT_ISTATUS_INTERRUPTED))
        opresult = LMNT_INTERRUPTED;
end:
    ctx->cur_instr = instr;
    return opresult;

branchcheck:
    if (opresult == LMNT_BRANCHING) {
        // the context's instruction pointer has been updated, refresh
        instr = ctx->cur_instr - 1; // will be incremented momentarily
        opresult = LMNT_OK;
    }
    goto dispatch;
}

LMNT_ATTR_FAST static inline LMNT_FORCEINLINE lmnt_result interrupt_function(lmnt_ictx* ctx)
{
    ctx->status_flags |= LMNT_ISTATUS_INTERRUPTED;
    return LMNT_OK;
}

#endif
