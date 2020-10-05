#pragma once

#include "element/common.h"
#include "element/interpreter.h"
#include "element/lmnt.h"
#include "instruction_tree/instructions.hpp"

#pragma warning(push)
#pragma warning(disable : 26819)
#include "lmnt/opcodes.h"
#include "lmnt/archive.h"
#pragma warning(pop)

#include <vector>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <cstring>

struct element_lmnt_stack_entry
{
    enum class entry_type
    {
        constant,
        input,
        output,
        local
    };
    entry_type type;
    size_t index;
    size_t size;
};

struct element_lmnt_instruction
{
    lmnt_opcode opcode;
    element_lmnt_stack_entry arg1;
    element_lmnt_stack_entry arg2;
    element_lmnt_stack_entry arg3;
};

struct element_lmnt_compiled_function
{
    const element_function* function;
    std::vector<element_lmnt_instruction> ops;
    size_t inputs_size = 0;
    size_t outputs_size = 0;
    size_t outputs_matched = 0;
    size_t locals_size = 0;
};

struct element_lmnt_archive_ctx
{
    std::vector<element_lmnt_compiled_function> functions;
    std::vector<lmnt_value> constants;
    std::unordered_map<const element_lmnt_compiled_function*, std::unordered_map<const element_instruction*, element_lmnt_stack_entry>> entries;

    size_t get_constant(lmnt_value v)
    {
        auto it = std::find(constants.begin(), constants.end(), v);
        if (it != constants.end())
            return std::distance(constants.begin(), it);
        constants.push_back(v);
        return constants.size() - 1;
    }
};
