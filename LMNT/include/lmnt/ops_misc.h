#ifndef LMNT_OPS_MISC_H
#define LMNT_OPS_MISC_H

#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/interpreter.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_assignss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignsv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigniis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignibs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigniiv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignibv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_minss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_minvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_maxss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_maxvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_minvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_maxvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_indexdis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_indexsss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_indexdss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_indexdsd(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_interrupt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

#endif
