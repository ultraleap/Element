#ifndef LMNT_OPS_BRANCH_H
#define LMNT_OPS_BRANCH_H

#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/interpreter.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_return(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_cmp(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_branch(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_brancheq(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchne(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchlt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchle(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchgt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchge(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchun(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_branchz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchnz(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchpos(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_branchneg(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

#endif