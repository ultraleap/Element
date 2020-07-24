#pragma once

//STD
#include <memory>

//SELF
#include "ast/fwd.hpp"
#include "fwd.hpp"

namespace element
{
	std::unique_ptr<scope> build_root_scope(const element_interpreter_ctx* context, const element_ast* ast, element_result& output_result);
}

