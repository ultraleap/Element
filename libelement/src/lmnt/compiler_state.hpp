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

struct compiler_state;

struct compiler_stack_info
{
    compiler_stack_info(const compiler_state& state) : _state(state) {}

    element_result set_allocation(const element::instruction* in, uint16_t count);
    element_result set_allocation_parent(const element::instruction* child, const element::instruction* parent, uint16_t rel_index);
    element_result clear_allocation(const element::instruction* in);
    element_result use_pinned_allocation(const element::instruction* in, uint16_t index, uint16_t count);
    element_result use_allocation(const element::instruction* current_in, const element::instruction* alloc_in);
    element_result get_stack_index(const element::instruction* in, uint16_t& index);
    element_result set_stack_index(const element::instruction* in, uint16_t index);

private:
    struct stack_allocation
    {
        const element::instruction* originating_instruction;
        uint16_t count;
        size_t set_instruction;
        size_t last_used_instruction;

        std::shared_ptr<stack_allocation> parent = nullptr;
        uint16_t stack_rel_index = 0;
        bool pinned = false;
        uint16_t pinned_index = 0;

        uint16_t stack_index() const
        {
            return stack_rel_index + (parent ? parent->stack_index() : 0);
        }
    };

    const compiler_state& _state;
    std::unordered_map<const element::instruction*, std::shared_ptr<stack_allocation>> _allocations;
};

struct compiler_state
{
    compiler_state(const element_lmnt_compiler_ctx& c, std::vector<element_value> v, uint16_t icount)
        : ctx(c)
        , constants(std::move(v))
        , inputs_count(icount)
        , stack(*this)
    {
    }

    const element_lmnt_compiler_ctx& ctx;
    std::vector<element_value> constants;
    uint16_t inputs_count;

    compiler_stack_info stack;
    std::unordered_map<const element::instruction*, size_t> results;
    std::vector<virtual_result> virtual_results;
    std::unordered_map<element_value, size_t> candidate_constants;

    element_result add_constant(element_value value, uint16_t* index = nullptr);
    element_result find_constant(element_value value, uint16_t& index);

    element_result find_virtual_result(const element::instruction* in, virtual_result& vr);
};