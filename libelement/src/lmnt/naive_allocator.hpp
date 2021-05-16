#pragma once

#include "lmnt/compiler_state.hpp"

class naive_allocator : public compiler_state::stack_allocator
{
    element_result allocate(const element::instruction* in) override
    {
        stack_allocation* alloc = get(in);
        if (!alloc)
        {
            return ELEMENT_ERROR_NOT_FOUND;
        }
        // for inputs/outputs/constants, nothing to do
        if (alloc->type() != allocation_type::local || alloc->pinned())
        {
            return ELEMENT_OK;
        }
        // if this has a parent, leave it to that
        if (alloc->parent)
        {
            return ELEMENT_OK;
        }

        alloc->rel_index = cur_index;
        cur_index += alloc->count;
        return ELEMENT_OK;
    }

private:
    uint16_t cur_index = 0;
};