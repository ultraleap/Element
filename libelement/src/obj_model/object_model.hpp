#pragma once

//STD
#include <memory>

//SELF
#include "ast/fwd.hpp"
#include "fwd.hpp"

namespace element
{
	std::shared_ptr<declaration> build_struct_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_constraint_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_function_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_namespace_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope);
	std::shared_ptr<declaration> build_declaration(const element_interpreter_ctx* context, const element_ast* ast, const scope* parent_scope);
	void build_expression(const element_interpreter_ctx* context, const element_ast* ast, expression_chain* chain);
	std::unique_ptr<scope> build_root_scope(const element_interpreter_ctx* context, const element_ast* ast);
}

