#pragma once

#include <memory>
#include "common_internal.hpp"

struct element_expression;
using expression_shared_ptr = std::shared_ptr<element_expression>;
using expression_const_shared_ptr = std::shared_ptr<const element_expression>;
