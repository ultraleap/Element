#pragma once

#include <memory>

struct element_expression;
struct element_compiler_ctx;

using expression_const_shared_ptr = std::shared_ptr<const element_expression>;