#ifndef LMNT_EXTCALLS_H
#define LMNT_EXTCALLS_H

#include <stdlib.h>
#include "lmnt/common.h"

struct lmnt_extcall_info;
typedef struct lmnt_extcall_info lmnt_extcall_info;

typedef lmnt_result(*lmnt_extcall_fn)(lmnt_ictx* ctx, const lmnt_extcall_info* callinfo, const lmnt_value* args, lmnt_value* rvals);

struct lmnt_extcall_info
{
    const char* name;
    lmnt_offset args_count;
    lmnt_offset rvals_count;
    lmnt_extcall_fn function;
};

lmnt_result lmnt_ictx_extcalls_get(const lmnt_ictx* ctx, const lmnt_extcall_info** table, size_t* table_count);
lmnt_result lmnt_ictx_extcalls_set(lmnt_ictx* ctx, const lmnt_extcall_info* table, size_t table_count);

lmnt_result lmnt_ictx_extcall_get(const lmnt_ictx* ctx, size_t index, const lmnt_extcall_info** result);
lmnt_result lmnt_ictx_extcall_find(const lmnt_ictx* ctx, const char* name, lmnt_offset args_count, lmnt_offset rvals_count, const lmnt_extcall_info** result);
lmnt_result lmnt_extcall_find_index(const lmnt_extcall_info* table, size_t table_size, const char* name, lmnt_offset args_count, lmnt_offset rvals_count, size_t* index);

#endif
