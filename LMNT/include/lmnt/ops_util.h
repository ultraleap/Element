#ifndef LMNT_OPS_UTIL_H
#define LMNT_OPS_UTIL_H

#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/interpreter.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_abss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_absv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_sumv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_floors(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_floorv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_rounds(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_roundv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_ceils(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_ceilv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

LMNT_ATTR_FAST lmnt_result lmnt_op_minss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_minvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_maxss(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_maxvv(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_minvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);
LMNT_ATTR_FAST lmnt_result lmnt_op_maxvs(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);

#endif