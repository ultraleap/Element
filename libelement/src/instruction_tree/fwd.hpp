#pragma once

#include <memory>

//TODO: unused, probably delete?
struct element_compiler_ctx;

namespace element
{
    struct instruction;

    using instruction_const_shared_ptr = std::shared_ptr<const instruction>;
} // namespace element