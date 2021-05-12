#include "lmnt/compiler_state.hpp"

element_result compiler_state::set_allocation(const element::instruction* in, uint16_t count)
{
    size_t inst_idx = results.at(in);
    auto result = _allocations.try_emplace(in, std::make_shared<stack_allocation>(stack_allocation{in, count, inst_idx, inst_idx}));
    return (result.second) ? ELEMENT_OK : ELEMENT_ERROR_UNKNOWN;
}

element_result compiler_state::set_allocation_if_not_pinned(const element::instruction* in, uint16_t count)
{
    if (set_allocation(in, count) == ELEMENT_OK)
        return ELEMENT_OK;
    auto it = _allocations.find(in);
    if (it != _allocations.end() && it->second->count == count && it->second->pinned)
        return ELEMENT_OK;
    return ELEMENT_ERROR_UNKNOWN;
}

element_result compiler_state::clear_allocation(const element::instruction* in)
{
    return _allocations.erase(in) ? ELEMENT_OK : ELEMENT_ERROR_NOT_FOUND;
}

element_result compiler_state::set_allocation_parent(const element::instruction* child, const element::instruction* parent, uint16_t rel_index)
{
    auto child_it = _allocations.find(child);
    if (child_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    auto parent_it = _allocations.find(parent);
    if (parent_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    // TODO: ensure child isn't pinned
    // ensure we actually fit in our new parent allocation
    if (child_it->second->count > parent_it->second->count) return ELEMENT_ERROR_INVALID_OPERATION;
    child_it->second->parent = parent_it->second;
    child_it->second->rel_index = rel_index;
    return ELEMENT_OK;
}

element_result compiler_state::use_allocation(const element::instruction* current_in, const element::instruction* alloc_in)
{
    auto it = _allocations.find(alloc_in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;

    it->second->last_used_instruction = results.at(current_in);
    return ELEMENT_OK;
}

element_result compiler_state::use_pinned_allocation(const element::instruction* in, allocation_type type, uint16_t index, uint16_t count)
{
    const size_t inst_idx = results.at(in);
    auto it = _allocations.try_emplace(in, std::make_shared<stack_allocation>(stack_allocation{in, count, 0, inst_idx, nullptr, type, index, true}));
    if (!it.first->second->pinned || it.first->second->cur_type != type || it.first->second->rel_index != index || it.first->second->count != count)
        return ELEMENT_ERROR_UNKNOWN;
    it.first->second->last_used_instruction = inst_idx;
    return ELEMENT_OK;
}

element_result compiler_state::calculate_stack_index(const element::instruction* in, uint16_t& index) const
{
    auto it = _allocations.find(in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    index = calculate_stack_index(it->second->type(), it->second->index());
    return ELEMENT_OK;
}

uint16_t compiler_state::calculate_stack_index(const allocation_type type, uint16_t index) const
{
    // if constant: index is already correct
    if (type == allocation_type::constant)
        return index;
    // otherwise add constants count
    index += static_cast<uint16_t>(constants.size());
    // inputs are directly after constants
    if (type == allocation_type::input)
        return index;
    // otherwise add inputs count
    index += inputs_count;
    // outputs are after inputs
    if (type == allocation_type::output)
        return index;
    // otherwise add outputs count
    index += virtual_results[results.at(return_instruction)].count;
    // local stack index
    return index;
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

element_result compiler_state::find_constant(element_value value, uint16_t& index) const
{
    if (auto it = std::find(constants.begin(), constants.end(), value); it != constants.end()) {
        index = static_cast<uint16_t>(std::distance(constants.begin(), it));
        return ELEMENT_OK;
    } else {
        return ELEMENT_ERROR_NOT_FOUND;
    }
}

element_result compiler_state::find_virtual_result(const element::instruction* in, virtual_result& vr) const
{
    if (auto it = results.find(in); it != results.end() && it->second < virtual_results.size()) {
        vr = virtual_results[it->second];
        return ELEMENT_OK;
    } else {
        return ELEMENT_ERROR_NOT_FOUND;
    }
}

uint16_t compiler_state::get_max_stack_usage() const
{
    uint16_t cur = 0;
    for (const auto& alloc : _allocations) {
        if (alloc.second->type() == allocation_type::local) {
            cur = uint16_t((std::max)(cur, uint16_t(alloc.second->index() + alloc.second->count)));
        }
    }
    return cur;
}
