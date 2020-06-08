#pragma once

#include <memory>

#include "element/ast.h"
#include "scopes/scope.hpp"

namespace element
{
	std::unique_ptr<declaration> build_struct_declaration(element_ast * ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_constraint_declaration(element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_function_declaration(element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_namespace_declaration(element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_declaration(element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::shared_ptr<root_scope> build_root_scope(element_ast* ast);
}

