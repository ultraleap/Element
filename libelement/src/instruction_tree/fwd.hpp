#pragma once

#include <memory>

struct element_instruction;
struct element_compiler_ctx;

using instruction_const_shared_ptr = std::shared_ptr<const element_instruction>;