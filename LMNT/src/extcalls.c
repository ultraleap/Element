#include "lmnt/extcalls.h"
#include "lmnt/interpreter.h"
#include <string.h>

lmnt_result lmnt_ictx_extcalls_get(const lmnt_ictx* ctx, const lmnt_extcall_info** table, size_t* table_count)
{
    *table = ctx->extcalls;
    *table_count = ctx->extcalls_count;
    return LMNT_OK;
}

lmnt_result lmnt_ictx_extcalls_set(lmnt_ictx* ctx, const lmnt_extcall_info* table, size_t table_count)
{
    ctx->extcalls = table;
    ctx->extcalls_count = table_count;
    return LMNT_OK;
}

lmnt_result lmnt_ictx_extcall_get(const lmnt_ictx* ctx, size_t index, const lmnt_extcall_info** result)
{
    if (index >= ctx->extcalls_count)
        return LMNT_ERROR_NOT_FOUND;
    *result = &ctx->extcalls[index];
    return LMNT_OK;
}

lmnt_result lmnt_ictx_extcall_find(const lmnt_ictx* ctx, const char* name, lmnt_offset args_count, lmnt_offset rvals_count, const lmnt_extcall_info** result)
{
    size_t index;
    LMNT_OK_OR_RETURN(lmnt_extcall_find_index(ctx->extcalls, ctx->extcalls_count, name, args_count, rvals_count, &index));
    *result = &ctx->extcalls[index];
    return LMNT_OK;
}

lmnt_result lmnt_extcall_find_index(const lmnt_extcall_info* table, size_t table_count, const char* name, lmnt_offset args_count, lmnt_offset rvals_count, size_t* index)
{
    for (size_t i = 0; i < table_count; ++i)
    {
        if (strcmp(name, table[i].name) == 0 && args_count == table[i].args_count && rvals_count == table[i].rvals_count)
        {
            *index = i;
            return LMNT_OK;
        }
    }
    return LMNT_ERROR_NOT_FOUND;
}
