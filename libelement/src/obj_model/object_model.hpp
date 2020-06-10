#pragma once

#include <memory>

#include "element/ast.h"
#include "scopes/scope.hpp"

namespace element
{
	std::unique_ptr<declaration> build_struct_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_constraint_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_function_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_scope_bodied_function_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_expression_bodied_function_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_namespace_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::unique_ptr<declaration> build_declaration(const element_ast* ast, const std::shared_ptr<scope>& parent_scope);
	std::shared_ptr<expression> build_expression(const element_ast* ast, std::shared_ptr<expression> parent);
	std::shared_ptr<root_scope> build_root_scope(const element_ast* ast);
}

