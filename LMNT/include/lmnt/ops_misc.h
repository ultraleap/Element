#ifndef LMNT_OPS_MISC_H
#define LMNT_OPS_MISC_H

#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/interpreter.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_noop(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_assignss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignsv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigniis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignibs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assigniiv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_assignibv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_dloadiis(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_dloadiiv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_dloadirs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_dloadirv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_dseclen(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_indexris(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_indexrir(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_interrupt(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

#endif
