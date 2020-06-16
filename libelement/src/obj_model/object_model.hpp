#pragma once

//STD
#include <memory>

//SELF
#include "element/ast.h"
#include "scope.hpp"

namespace element
{
	std::shared_ptr<declaration> build_struct_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_constraint_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_function_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_scope_bodied_function_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_expression_bodied_function_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_namespace_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_declaration(const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<expression> build_expression(const element_ast* ast, std::shared_ptr<element::expression> parent);
	std::unique_ptr<scope> build_root_scope(const element_ast* ast);
}

