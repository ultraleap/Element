#include <algorithm>
#include <vector>
#include <unordered_map>

#include "instruction_tree/instructions.hpp"
#include "lmnt/compiler.hpp"

struct virtual_result
{
    const element::instruction* instruction = nullptr;
    uint16_t count = 0;
    bool prepared = false;
    bool is_output = false;
};

enum class allocation_type
{
    constant,
    input,
    output,
    local
};

struct compiler_state
{
    compiler_state(const element_lmnt_compiler_ctx& c, const element::instruction* in, std::vector<element_value>& v, uint16_t icount)
        : ctx(c)
        , return_instruction(in)
        , constants(v)
        , inputs_count(icount)
    {
    }

    const element_lmnt_compiler_ctx& ctx;
    const element::instruction* return_instruction;
    std::vector<element_value>& constants;
    uint16_t inputs_count;

    std::unordered_map<const element::instruction*, size_t> results;
    std::vector<virtual_result> virtual_results;
    std::unordered_map<element_value, size_t> candidate_constants;

    element_result add_constant(element_value value, uint16_t* index = nullptr);
    element_result find_constant(element_value value, uint16_t& index);

    element_result find_virtual_result(const element::instruction* in, virtual_result& vr);


    element_result set_allocation(const element::instruction* in, uint16_t count);
    element_result set_allocation_if_not_pinned(const element::instruction* in, uint16_t count);
    element_result set_allocation_parent(const element::instruction* child, const element::instruction* parent, uint16_t rel_index);
    element_result clear_allocation(const element::instruction* in);
    element_result use_pinned_allocation(const element::instruction* in, allocation_type type, uint16_t index, uint16_t count);
    element_result use_allocation(const element::instruction* current_in, const element::instruction* alloc_in);

    uint16_t get_stack_index(const allocation_type type, uint16_t index);
    element_result get_stack_index(const element::instruction* in, uint16_t& index);

private:
    struct stack_allocation
    {
        const element::instruction* originating_instruction;
        uint16_t count;
        size_t set_instruction;
        size_t last_used_instruction;

        std::shared_ptr<stack_allocation> parent = nullptr;
        allocation_type cur_type = allocation_type::local;
        uint16_t rel_index = 0;
        bool pinned = false;

        allocation_type type() const
        {
            return (parent) ? parent->type() : cur_type;
        }

        uint16_t index() const
        {
            return rel_index + (parent ? parent->index() : 0);
        }
    };

    std::unordered_map<const element::instruction*, std::shared_ptr<stack_allocation>> _allocations;
};