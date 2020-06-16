#include "declarations.hpp"

//SELF
#include "obj_model/scope.hpp"
#include "obj_model/expressions.hpp"
#include "obj_model/intermediaries.hpp"
#include "etree/expressions.hpp"

//declaration
element::declaration::declaration(element::identifier identifier)
	: output{ nullptr }, identifier(std::move(identifier))
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
	return identifier.value;
}

//scoped_declaration
element::scoped_declaration::scoped_declaration(element::identifier identifier, const element::scope* parent_scope)
    : declaration(std::move(identifier))
{
	scope = std::make_unique<element::scope>(parent_scope, this);
}

bool element::scoped_declaration::has_scope() const
{
	return !scope->is_empty();
}

void element::scoped_declaration::add_declaration(std::shared_ptr<element::declaration> declaration) const
{
	scope->add_declaration(std::move(declaration));
}

std::string element::scoped_declaration::location() const
{
	auto declaration = identifier.value;

	if (scope->get_parent_scope()->is_root())
		return declaration;

	//recursive construction
	return scope->get_parent_scope()->location() + "." + declaration;
}

//struct
element::struct_declaration::struct_declaration(element::identifier identifier, const element::scope* parent_scope, const bool is_intrinsic)
	: scoped_declaration(std::move(identifier), parent_scope)
{
	qualifier = struct_qualifier;
	intrinsic = is_intrinsic;
}

std::string element::struct_declaration::to_string() const
{
	return location() + ":Struct";
}

std::string element::struct_declaration::to_code(const int depth) const
{
	std::string ports;

	const std::string offset = "    ";
	std::string declaration_offset;

	for (auto i = 0; i < depth; ++i)
		declaration_offset += offset;

	if (has_inputs()) {
		static auto accumulate = [depth](std::string accumulator, const element::port& port)
		{
			return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
		};

		const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
		ports = "(" + input_ports + ")";
	}

	if (intrinsic)
		return declaration_offset + "intrinsic struct " + identifier.value + ports + scope->to_code(depth);

	return declaration_offset + "struct "+ identifier.value + ports + scope->to_code(depth);
}

std::shared_ptr<element::element_object> element::struct_declaration::index(const element::identifier& identifier) const
{
	return scope->find(identifier.value, false);
}

 //constraint
element::constraint_declaration::constraint_declaration(element::identifier identifier, const bool is_intrinsic)
    : declaration(std::move(identifier))
{
	qualifier = constraint_qualifier;
	intrinsic = is_intrinsic;
}

std::string element::constraint_declaration::to_string() const
{
	return location() + ":Constraint";
}

std::string element::constraint_declaration::to_code(const int depth) const
{
	std::string ports;

	const std::string offset = "    ";
	std::string declaration_offset;

	for (auto i = 0; i < depth; ++i)
		declaration_offset += offset;

	if (has_inputs()) {
		static auto accumulate = [depth](std::string accumulator, const element::port& port)
		{
			return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
		};

		const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
		ports = "(" + input_ports + ")";
	}

	if (output)
		ports += output->to_code(depth);

	if (intrinsic)
		return declaration_offset + "intrinsic constraint " + identifier.value + ports;

	return declaration_offset + "constraint " + identifier.value + ports;
}

//function
element::function_declaration::function_declaration(element::identifier identifier, const element::scope* parent_scope, const bool is_intrinsic)
	: scoped_declaration(std::move(identifier), parent_scope)
{
	qualifier = function_qualifier;
	intrinsic = is_intrinsic;
}

std::string element::function_declaration::to_string() const
{
	return location() + ":Function";
}

std::string element::function_declaration::to_code(int depth) const
{
	auto declaration = identifier.value;
	std::string ports;

	const std::string offset = "    ";
	std::string declaration_offset;

	for (auto i = 0; i < depth; ++i)
		declaration_offset += offset;

	if (has_inputs()) {
		static auto accumulate = [depth](std::string accumulator, const element::port& port)
		{
			return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
		};

        const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
		ports = "(" + input_ports + ")";
	}

	if (output)
		ports += output->to_code(depth);

	if (intrinsic)
	    return declaration_offset + "intrinsic " + identifier.value + ports + ";";

    return declaration_offset + identifier.value + ports + scope->to_code(depth);
}

std::shared_ptr<element::element_object> element::function_declaration::call(std::vector<std::shared_ptr<compiled_expression>> args) const
{
	//e.g. my_namespace.my_function(1) is a call on a function declaration

	if (inputs.size() == args.size())
	{
		//todo: cache arguments as part of callstack for our child functions (e.g. return) to be able to reference
		//todo: forward on any extra arguments to return? return can have it's own portlist right? how do we determine that, hmm, it would be here though
		if (intrinsic)
		{
			auto result = std::make_shared<compiled_expression>();
			result->declarer = this;
			//todo: better way of getting to our parent?
			if (scope->get_parent_scope()->declarer->identifier.value == "Num" && identifier.value == "add")
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
element::expression_bodied_function_declaration::expression_bodied_function_declaration(element::identifier identifier, const element::scope* parent_scope)
	: scoped_declaration(std::move(identifier), parent_scope)
{
	qualifier = function_qualifier;
	intrinsic = false;
}

std::string element::expression_bodied_function_declaration::to_string() const
{
	return location() + ":Function";
}

std::string element::expression_bodied_function_declaration::to_code(const int depth) const
{
	auto declaration = identifier.value;

	const std::string offset = "    ";
	std::string declaration_offset;

	for (auto i = 0; i < depth; ++i)
		declaration_offset += offset;

	if (has_inputs()) {
		static auto accumulate = [depth](std::string accumulator, const element::port& port)
		{
			return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
		};

		const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
		declaration = identifier.value + "(" + input_ports + ")";
	}

	if (output)
		declaration += output->to_code(depth);

	return declaration_offset + declaration + " = " + expression->to_code() + ";";
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
element::namespace_declaration::namespace_declaration(element::identifier identifier, const element::scope* parent_scope)
	: scoped_declaration(std::move(identifier), parent_scope)
{
	qualifier = namespace_qualifier;
	intrinsic = false;
}

std::string element::namespace_declaration::to_string() const
{
	return location() + ":Namespace";
}

std::string element::namespace_declaration::to_code(const int depth) const
{
	return "namespace " + identifier.value + scope->to_code(depth);
}

std::shared_ptr<element::element_object> element::namespace_declaration::index(const element::identifier& expr) const
{
	return scope->find(identifier.value, false);
}
