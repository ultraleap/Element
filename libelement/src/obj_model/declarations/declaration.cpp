#include "declaration.hpp"

//SELF
#include "../scopes/scope.hpp"
#include "../expressions/expression.hpp"

//declaration
element::declaration::declaration()
	: output{ nullptr }
{
}

bool element::declaration::is_intrinsic() const
{
	return intrinsic;
}

bool element::declaration::has_inputs() const
{
	return !inputs.empty();
}

bool element::declaration::has_output() const
{
	return output != nullptr;
}

bool element::declaration::has_constraint() const
{
	//TODO: Constraints
	return false; //return constraint != nullptr;
}

std::string element::declaration::location() const
{
	return identifier;
}

//scoped_declaration
element::scoped_declaration::scoped_declaration(const element::scope* parent_scope)
{
	scope = std::make_unique<element::scope>(parent_scope, this);
}

bool element::scoped_declaration::has_scope() const
{
	return !scope->declarations.empty();
}

void element::scoped_declaration::add_declaration(std::unique_ptr<element::declaration> declaration) const
{
	scope->declarations.emplace(declaration->identifier, std::move(declaration));
}

std::string element::scoped_declaration::location() const
{
	auto declaration = identifier;

	if (scope->parent_scope->is_root())
		return declaration;

	//recursive construction
	return scope->parent_scope->location() + "." + declaration;
}

//struct
element::struct_declaration::struct_declaration(const element::scope* parent_scope, bool is_intrinsic)
	: scoped_declaration(parent_scope)
{
	qualifier = struct_qualifier;
	intrinsic = is_intrinsic;
}

std::string element::struct_declaration::to_string() const
{
	std::string ports;

	if (has_inputs()) {
		static auto accumulate = [](std::string accumulator, const element::port& port)
		{
			return std::move(accumulator) + ", " + port.to_string();
		};

		const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].identifier, accumulate);
		ports = "(" + input_ports + ")";
	}

	return location() + ports + ":Struct";
}

const element::element_object* element::struct_declaration::index(const indexing_expression* expr) const
{
	return scope->find(expr->identifier, false);
}

 //constraint
element::constraint_declaration::constraint_declaration(bool is_intrinsic)
{
	qualifier = constraint_qualifier;
	intrinsic = is_intrinsic;
}


//function
element::function_declaration::function_declaration(const element::scope* parent_scope, bool is_intrinsic)
	: scoped_declaration(parent_scope)
{
	qualifier = function_qualifier;
	intrinsic = is_intrinsic;
}

std::string element::function_declaration::to_string() const
{
	auto declaration = identifier;

	if (has_inputs()) {
		static auto accumulate = [](std::string accumulator, const element::port& port)
		{
			return std::move(accumulator) + ", " + port.to_string();
		};
	
		const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].identifier, accumulate);
		declaration = identifier + "(" + input_ports + ")";
	}
	
	return location() + ":Function";
}

const element::element_object* element::function_declaration::call(const call_expression* expr) const
{
	//:b
	if (intrinsic && identifier == "add")
	{
		auto result = std::make_shared<compiled_expression>();
	}

	return nullptr;
}

//expression bodied function
element::expression_bodied_function_declaration::expression_bodied_function_declaration(const element::scope* parent_scope)
	: scoped_declaration(parent_scope)
{
	qualifier = function_qualifier;
	intrinsic = false;
}

std::string element::expression_bodied_function_declaration::to_string() const
{
	return location() + " = " + expression->to_string() + ":Function";
}

std::shared_ptr<element::compiled_expression> element::expression_bodied_function_declaration::compile() const
{
	return expression->compile();
}

//namespace
element::namespace_declaration::namespace_declaration(const element::scope* parent_scope)
	: scoped_declaration(parent_scope)
{
	qualifier = namespace_qualifier;
	intrinsic = false;
}

std::string element::namespace_declaration::to_string() const
{
	return location() + ":Namespace";
}

const element::element_object* element::namespace_declaration::index(const indexing_expression* expr) const
{
	return scope->find(expr->identifier, false);
}
