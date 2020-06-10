#include "declaration.hpp"

#include "../scopes/scope.hpp"

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
	return constraint != nullptr;
}

std::string element::declaration::location() const
{
	return identifier;
}

//scoped_declaration
element::scoped_declaration::scoped_declaration(const std::shared_ptr<element::scope>& parent_scope) : scope{ parent_scope }
{
	scope = std::make_shared<element::scope>(parent_scope, this);
}

bool element::scoped_declaration::has_scope() const
{
	return !scope->declarations.empty();
}

void element::scoped_declaration::add_declaration(std::unique_ptr<element::declaration> declaration) const
{
	scope->declarations.push_back(std::move(declaration));
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
element::struct_declaration::struct_declaration(const std::shared_ptr<element::scope>& parent_scope, bool is_intrinsic)
	: scoped_declaration(parent_scope)
{
	qualifier = struct_qualifier;
	intrinsic = is_intrinsic;
}

std::string element::struct_declaration::to_string() const
{
	return location() + ":Struct";
}

 //constraint
element::constraint_declaration::constraint_declaration(bool is_intrinsic)
{
	qualifier = constraint_qualifier;
	intrinsic = is_intrinsic;
}


//function
element::function_declaration::function_declaration(const std::shared_ptr<element::scope>& parent_scope, bool is_intrinsic)
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
	
	return location() + declaration + ":Function";
}

//expression bodied function
element::expression_bodied_function_declaration::expression_bodied_function_declaration(const std::shared_ptr<element::scope>& parent_scope)
	: scoped_declaration(parent_scope)
{
	qualifier = function_qualifier;
	intrinsic = false;
}

std::string element::expression_bodied_function_declaration::to_string() const
{
	return location() + " = " + expression->to_string() + ":Function";
}

//namespace
element::namespace_declaration::namespace_declaration(const std::shared_ptr<element::scope>& parent_scope)
	: scoped_declaration(parent_scope)
{
	qualifier = namespace_qualifier;
	intrinsic = false;
}

std::string element::namespace_declaration::to_string() const
{
	return location() + ":Namespace";
}