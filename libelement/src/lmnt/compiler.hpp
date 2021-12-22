#pragma once

#include <string>
#include <vector>
#include "instruction_tree/instructions.hpp"
#include "lmnt/archive.h"

struct element_lmnt_compiler_optimisers
{
    // bool cse; // required
    size_t constant_reuse_threshold = 1;
    bool minimise_moves = true;
    bool stack_reuse = true;
    bool allow_dynamic = true;
};

struct element_lmnt_compiler_settings
{
    bool allow_backbranches = true;
    bool allow_dynamic = true;
};

struct element_lmnt_compiler_ctx
{
    element_lmnt_compiler_settings settings;
    element_lmnt_compiler_optimisers optimise;
};

struct element_lmnt_compiled_function
{
    std::string name;
    std::vector<lmnt_instruction> instructions;
    size_t local_stack_count = 0;
    size_t inputs_count = 0;
    size_t outputs_count = 0;
    lmnt_def_flags flags = LMNT_DEFFLAG_NONE;

    size_t total_stack_count() const { return inputs_count + outputs_count + local_stack_count; }
};

element_result element_lmnt_find_constants(
    const element_lmnt_compiler_ctx& ctx,
    const element::instruction_const_shared_ptr& instruction,
    std::unordered_map<element_value, size_t>& candidates);

element_result element_lmnt_compile_function(
    const element_lmnt_compiler_ctx& ctx,
    const element::instruction_const_shared_ptr instruction,
    std::string name,
    std::vector<element_value>& constants,
    const size_t inputs_count,
    element_lmnt_compiled_function& output);
