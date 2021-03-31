#ifndef LMNT_OPS_FNCALL_H
#define LMNT_OPS_FNCALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/interpreter.h"

LMNT_ATTR_FAST lmnt_result lmnt_op_extcall(lmnt_ictx* ctx, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3);


#ifdef __cplusplus
}
#endif

#endif