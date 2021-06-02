#include "lmnt/compiler_state.hpp"
#include "lmnt/naive_allocator.hpp"

compiler_state::compiler_state(const element_lmnt_compiler_ctx& c, const element::instruction* in, std::vector<element_value>& v, uint16_t icount)
    : ctx(c)
    , return_instruction(in)
    , constants(v)
    , inputs_count(icount)
{
    // TODO: better allocators
    allocator = std::make_unique<naive_allocator>();
}

stack_allocation* compiler_state::stack_allocator::get(const element::instruction* in)
{
    auto it = _allocations.find(in);
    return (it != _allocations.end()) ? it->second : nullptr;
}

const stack_allocation* compiler_state::stack_allocator::get(const element::instruction* in) const
{
    auto it = _allocations.find(in);
    return (it != _allocations.end()) ? it->second : nullptr;
}

element_result compiler_state::stack_allocator::get_type(const element::instruction* in, allocation_type& type) const
{
    auto it = _allocations.find(in);
    if (it != _allocations.end())
    {
        type = it->second->type();
        return ELEMENT_OK;
    }
    return ELEMENT_ERROR_NOT_FOUND;
}

bool compiler_state::stack_allocator::is_type(const element::instruction* in, allocation_type type) const
{
    auto it = _allocations.find(in);
    return (it != _allocations.end() && it->second->type() == type);
}

element_result compiler_state::stack_allocator::add(const element::instruction* in, size_t inst_idx, uint16_t count, stack_allocation** vr)
{
    auto allocation = std::make_shared<stack_allocation>(in, count, inst_idx, inst_idx);
    auto result = _allocations.try_emplace(in, allocation.get());
    if (vr) *vr = result.first->second;
    if (result.second)
    {
        _alloc_storage.emplace_back(std::move(allocation));
        return ELEMENT_OK;
    }
    else
    {
        return ELEMENT_ERROR_UNKNOWN;
    }
}

element_result compiler_state::stack_allocator::set_count(const element::instruction* in, uint16_t count)
{
    auto it = _allocations.find(in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;

    it->second->count = count;
    return ELEMENT_OK;
}

element_result compiler_state::stack_allocator::erase(const element::instruction* in)
{
    size_t count = _allocations.erase(in);
    _alloc_storage.erase(
        std::find_if(_alloc_storage.begin(), _alloc_storage.end(), [=](const auto& sa) { return sa->instruction == in; }));
    return (count) ? ELEMENT_OK : ELEMENT_ERROR_NOT_FOUND;
}

element_result compiler_state::stack_allocator::set_parent(const element::instruction* child, const element::instruction* parent, uint16_t rel_index)
{
    auto child_it = _allocations.find(child);
    if (child_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    auto parent_it = _allocations.find(parent);
    if (parent_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    // cannot displace a pinned allocation
    if (child_it->second->pinned())
        return ELEMENT_ERROR_INVALID_OPERATION;
    // ensure we actually fit in our new parent allocation
    if (child_it->second->count > parent_it->second->count) return ELEMENT_ERROR_INVALID_OPERATION;
    child_it->second->parent = parent_it->second->shared_from_this();
    child_it->second->rel_index = rel_index;
    return ELEMENT_OK;
}

element_result compiler_state::stack_allocator::use(const element::instruction* current_in, const element::instruction* alloc_in)
{
    auto a_it = _allocations.find(alloc_in);
    if (a_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;
    auto c_it = _allocations.find(current_in);
    if (c_it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;

    a_it->second->last_used_instruction = c_it->second->set_instruction;
    return ELEMENT_OK;
}

element_result compiler_state::stack_allocator::set_pinned(const element::instruction* in, allocation_type type, uint16_t index, uint16_t count)
{
    auto it = _allocations.find(in);
    if (it == _allocations.end()) return ELEMENT_ERROR_NOT_FOUND;

    it->second->parent = nullptr;
    it->second->rel_type = type;
    it->second->rel_index = index;
    it->second->count = count;
    it->second->last_used_instruction = it->second->set_instruction;
    it->second->rel_pinned = true;
    return ELEMENT_OK;
}

element_result compiler_state::calculate_stack_index(const element::instruction* in, uint16_t& index) const
{
    stack_allocation* alloc = allocator->get(in);
    if (!alloc) return ELEMENT_ERROR_NOT_FOUND;
    index = calculate_stack_index(alloc->type(), alloc->index());
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
    index += allocator->get(return_instruction)->count;
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

uint16_t compiler_state::stack_allocator::get_max_stack_usage() const
{
    uint16_t cur = 0;
    for (const auto& alloc : _allocations) {
        if (alloc.second->type() == allocation_type::local) {
            cur = uint16_t((std::max)(cur, uint16_t(alloc.second->index() + alloc.second->count)));
        }
    }
    return cur;
}
