#include "declaration.hpp"

//SELF
#include "obj_model/scopes/scope.hpp"
#include "obj_model/expressions/expression.hpp"
#include "obj_model/intermediaries/struct_instance.hpp"
#include "etree/expressions.hpp"

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

void element::scoped_declaration::add_declaration(std::shared_ptr<element::declaration> declaration) const
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

std::shared_ptr<element::element_object> element::struct_declaration::index(const indexing_expression* expr) const
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

std::shared_ptr<element::element_object> element::function_declaration::call(std::vector<std::shared_ptr<compiled_expression>> args) const
{
	//e.g. Mynamespace.my_function(1) is a call on a function declaration

	if (inputs.size() == args.size())
	{
		//todo: cache arguments as part of callstack for our child functions (e.g. return) to be able to reference
		//todo: forward on any extra arguments to return? return can have it's own portlist right? how do we determine that, hmm, it would be here though
		if (intrinsic)
		{
			auto result = std::make_shared<compiled_expression>();
			result->declarer = this;
			//todo: better way of getting to our parent?
			if (scope->parent_scope->declarer->identifier == "Num" && identifier == "add")
			{
				result->expression = std::make_shared<element_expression_binary>(
					element_binary_op::add,
					args[0]->expression,
					args[1]->expression);
			}

			return result;
		}
		else
		{
			return scope->find("return", false)->call({});
		}
	}

	//don't have all the arguments, so partially apply them
	//is a compiled expression because if the call is the full expression, then we return it from expression->compile, which needs to be a compiled expression.
	//todo: need to figure out differences between compiling and calling and their restrictions, if any
	auto result = std::make_shared<compiled_expression>();
	result->declarer = this;
	result->object = std::make_shared<function_instance>(this, std::move(args));
	return std::move(result);
}

std::shared_ptr<element::compiled_expression> element::function_declaration::compile() const
{
	throw;
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

std::shared_ptr<element::element_object> element::expression_bodied_function_declaration::call(std::vector<std::shared_ptr<compiled_expression>> args) const
{
	//todo: apply arguments to callstack/cache so they can be found from scope lookups
	const auto& compiled = compile();
	//if it was a function instance then we should apply the arguments we have to it
    //e.g. evaluate = add5(2); will call function declaration add5 with 2, so we need to apply 2 here. maybe there's a better way
	if (dynamic_cast<function_instance*>(compiled->object.get()) &&
		static_cast<int>(args.size()) > static_cast<int>(inputs.size())) //we were given more args than we accept, so apply the remaining ones to the function instance
	{
		const auto& instance = static_cast<function_instance*>(compiled->object.get());
		instance->provided_arguments.insert(instance->provided_arguments.end(), args.begin() + inputs.size(), args.end());

		//egh.. it feels like everything needs to return a compiled_expression
		if (instance->provided_arguments.size() == instance->declarer->inputs.size())
			return instance->call({});
		else
			compiled->object = instance->call({});
	}

	return compiled;
}

std::shared_ptr<element::compiled_expression> element::expression_bodied_function_declaration::compile() const
{
	//todo: should compilation have to happen via a call? I think so, so this function becomes redundant/an issue? mmmm
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

std::shared_ptr<element::element_object> element::namespace_declaration::index(const indexing_expression* expr) const
{
	return scope->find(expr->identifier, false);
}
