#ifndef LMNT_OPS_BRANCH_H
#define LMNT_OPS_BRANCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/interpreter.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_return(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_cmp(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_cmpz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_branch(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_branchceq(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchcne(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchclt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchcle(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchcgt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchcge(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchcun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_branchz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchnz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchpos(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchneg(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_assignceq(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigncne(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignclt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigncle(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigncgt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigncge(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigncun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);


#ifdef __cplusplus
}
#endif

#endif