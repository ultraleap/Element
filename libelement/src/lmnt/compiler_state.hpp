#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>

#include "instruction_tree/instructions.hpp"
#include "lmnt/compiler.hpp"


enum class compilation_stage
{
    none,
    created,
    prepared,
    allocated,
    compiled
};

enum class allocation_type
{
    constant,
    input,
    output,
    local
};

struct stack_allocation : std::enable_shared_from_this<stack_allocation>
{
    stack_allocation(const element::instruction* in, uint16_t count, size_t set_in, size_t last_in)
        : instruction(in)
        , count(count)
        , set_instruction(set_in)
        , last_used_instruction(last_in)
    {
    }

    const element::instruction* instruction;
    uint16_t count;
    size_t set_instruction;
    size_t last_used_instruction;

    compilation_stage stage = compilation_stage::none;

    std::shared_ptr<stack_allocation> parent = nullptr;
    allocation_type rel_type = allocation_type::local;
    uint16_t rel_index = 0;
    bool rel_pinned = false;

    allocation_type type() const
    {
        return (parent) ? parent->type() : rel_type;
    }

    bool pinned() const
    {
        return (parent) ? parent->pinned() : rel_pinned;
    }

    uint16_t index() const
    {
        return rel_index + (parent ? parent->index() : 0);
    }
};


struct compiler_state
{
    compiler_state(const element_lmnt_compiler_ctx& c, const element::instruction* in, std::vector<element_value>& v, uint16_t icount);

    const element_lmnt_compiler_ctx& ctx;
    const element::instruction* return_instruction;
    std::vector<element_value>& constants;
    uint16_t inputs_count;

    size_t cur_instruction_index = 0;
    std::unordered_map<element_value, size_t> candidate_constants;

    element_result add_constant(element_value value, uint16_t* index = nullptr);
    element_result find_constant(element_value value, uint16_t& index) const;

    uint16_t calculate_stack_index(const allocation_type type, uint16_t index) const;
    element_result calculate_stack_index(const element::instruction* in, uint16_t& index, size_t alloc_index = 0) const;

public:
    struct stack_allocator
    {
        stack_allocation* get(const element::instruction* in, size_t alloc_index = 0);
        const stack_allocation* get(const element::instruction* in, size_t alloc_index = 0) const;
        size_t count(const element::instruction* in) const;
        element_result get_type(const element::instruction* in, allocation_type& type) const { return get_type(in, 0, type); }
        element_result get_type(const element::instruction* in, size_t alloc_index, allocation_type& type) const;
        bool is_type(const element::instruction* in, allocation_type type) const { return is_type(in, 0, type); }
        bool is_type(const element::instruction* in, size_t alloc_index, allocation_type type) const;
        element_result add(const element::instruction* in, uint16_t count, stack_allocation** result = nullptr);
        element_result set_count(const element::instruction* in, uint16_t count) { return set_count(in, 0, count); }
        element_result set_count(const element::instruction* in, size_t alloc_index, uint16_t count);
        element_result set_pinned(const element::instruction* in, allocation_type type, uint16_t index, uint16_t count) { return set_pinned(in, 0, type, index, count); }
        element_result set_pinned(const element::instruction* in, size_t alloc_index, allocation_type type, uint16_t index, uint16_t count);
        element_result set_parent(const element::instruction* child, const element::instruction* parent, uint16_t rel_index) { return set_parent(child, 0, parent, 0, rel_index); }
        element_result set_parent(const element::instruction* child, size_t child_alloc_index, const element::instruction* parent, size_t parent_alloc_index, uint16_t rel_index);
        element_result clear(const element::instruction* in);
        element_result use(const element::instruction* current_in, const element::instruction* alloc_in) { return use(current_in, 0, alloc_in, 0); }
        element_result use(const element::instruction* current_in, size_t current_index, const element::instruction* alloc_in, size_t alloc_index);
        element_result set_stage(const element::instruction* in, compilation_stage stage);

        uint16_t get_max_stack_usage() const;

        virtual element_result allocate(const element::instruction* in, size_t index) = 0;

        element_result allocate(const element::instruction* in)
        {
            for (size_t i = 0; i < count(in); ++i)
                ELEMENT_OK_OR_RETURN(allocate(in, i));
            return ELEMENT_OK;
        }

        // TODO: knowledge of conditional execution etc, use scopes?

        virtual ~stack_allocator() = default;
    protected:
        std::vector<std::shared_ptr<stack_allocation>> _alloc_storage;
        std::unordered_map<const element::instruction*, std::vector<stack_allocation*>> _allocations;
    };

    std::unique_ptr<stack_allocator> allocator;
};