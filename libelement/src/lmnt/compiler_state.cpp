#include "lmnt/compiler_state.hpp"

element_result compiler_stack_info::set_allocation(const element::instruction* in, uint16_t count)
{
    size_t inst_idx = _state.results.at(in);
    auto result = _allocations.try_emplace(in, std::make_shared<stack_allocation>(stack_allocation{in, count, inst_idx, inst_idx}));
    return (result.second) ? ELEMENT_OK : ELEMENT_ERROR_UNKNOWN;
}

element_result compiler_stack_info::clear_allocation(const element::instruction* in)
{
    return _allocations.erase(in) ? ELEMENT_OK : ELEMENT_ERROR_NOT_FOUND;
}

element_result compiler_stack_info::set_allocation_parent(const element::instruction* child, const element::instruction* parent, uint16_t rel_index)
{
    auto child_it = _allocations.find(child);
    if (child_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    auto parent_it = _allocations.find(parent);
    if (parent_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    // ensure we actually fit in our new parent allocation
    if (child_it->second->count > parent_it->second->count) return ELEMENT_ERROR_INVALID_OPERATION;
    child_it->second->parent = parent_it->second;
    child_it->second->stack_rel_index = rel_index;
    return ELEMENT_OK;
}

element_result compiler_stack_info::use_allocation(const element::instruction* current_in, const element::instruction* alloc_in)
{
    auto it = _allocations.find(alloc_in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;

    it->second->last_used_instruction = _state.results.at(current_in);
    return ELEMENT_OK;
}

element_result compiler_stack_info::use_pinned_allocation(const element::instruction* in, uint16_t index, uint16_t count)
{
    const size_t inst_idx = _state.results.at(in);
    auto it = _allocations.try_emplace(in, std::make_shared<stack_allocation>(stack_allocation{in, count, 0, inst_idx, nullptr, index, true, index}));
    if (!it.first->second->pinned || it.first->second->pinned_index != index || it.first->second->count != count)
        return ELEMENT_ERROR_UNKNOWN;
    it.first->second->last_used_instruction = inst_idx;
    return ELEMENT_OK;
}

element_result compiler_stack_info::get_stack_index(const element::instruction* in, uint16_t& index)
{
    auto it = _allocations.find(in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    index = it->second->stack_index();
    return ELEMENT_OK;
}

element_result compiler_stack_info::set_stack_index(const element::instruction* in, uint16_t index)
{
    auto it = _allocations.find(in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    it->second->parent = nullptr;
    it->second->stack_rel_index = index;
    return ELEMENT_OK;
}

element_result compiler_state::add_constant(element_value value, uint16_t* index)
{
    if (constants.size() >= UINT16_MAX)
        return ELEMENT_ERROR_UNKNOWN;
    auto it = std::find(constants.begin(), constants.end(), value);
    // if it didn't exist before, add it
    if (it == constants.end())
        constants.emplace_back(value);
    // if we're returning the index, use the iterator's position (which is now valid either way)
    if (index)
        *index = static_cast<uint16_t>(std::distance(constants.begin(), it));
    return ELEMENT_OK;
}

element_result compiler_state::find_constant(element_value value, uint16_t& index)
{
    if (auto it = std::find(constants.begin(), constants.end(), value); it != constants.end())
    {
        index = static_cast<uint16_t>(std::distance(constants.begin(), it));
        return ELEMENT_OK;
    }
    else
    {
        return ELEMENT_ERROR_NOT_FOUND;
    }
}

element_result compiler_state::find_virtual_result(const element::instruction* in, virtual_result& vr)
{
    if (auto it = results.find(in); it != results.end() && it->second < virtual_results.size())
    {
        vr = virtual_results[it->second];
        return ELEMENT_OK;
    }
    else
    {
        return ELEMENT_ERROR_NOT_FOUND;
    }
}