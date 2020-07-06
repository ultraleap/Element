#include "declarations.hpp"
#include "expressions.hpp"
#include "port.hpp"
#include "scope.hpp"
#include "intermediaries.hpp"

//to_string
std::string element::scope::to_string() const
{
    return declarer->to_string();
}

std::string element::struct_declaration::to_string() const
{
    return location() + ":Struct";
}

std::string element::constraint_declaration::to_string() const
{
    return location() + ":Constraint";
}

std::string element::function_declaration::to_string() const
{
    return location() + ":Function";
}

std::string element::namespace_declaration::to_string() const
{
    return location() + ":Namespace";
}

std::string element::struct_instance::to_string() const
{
    return "Instance:" + declarer->to_string();
}

std::string element::function_instance::to_string() const
{
    //todo: element test expects Function, not FunctionInstance
    //return declarer->location() + ":FunctionInstance";
    return declarer->location() + ":Function";
}

//to_code
std::string element::struct_declaration::to_code(const int depth) const
{
    std::string ports;

    const std::string offset = "    ";
    std::string declaration_offset;

    for (auto i = 0; i < depth; ++i)
        declaration_offset += offset;

    if (has_inputs()) {
        static auto accumulate = [depth](std::string accumulator, const port& port)
        {
            return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
        };

        const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
        ports = "(" + input_ports + ")";
    }

    if (_intrinsic)
        return declaration_offset + "intrinsic struct " + name.value + ports + our_scope->to_code(depth);

    return declaration_offset + "struct " + name.value + ports + our_scope->to_code(depth);
}

std::string element::constraint_declaration::to_code(const int depth) const
{
    std::string ports;

    const std::string offset = "    ";
    std::string declaration_offset;

    for (auto i = 0; i < depth; ++i)
        declaration_offset += offset;

    if (has_inputs()) {
        static auto accumulate = [depth](std::string accumulator, const port& port)
        {
            return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
        };

        const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
        ports = "(" + input_ports + ")";
    }

    if (output)
        ports += output->to_code(depth);

    if (_intrinsic)
        return declaration_offset + "intrinsic constraint " + name.value + ports;

    return declaration_offset + "constraint " + name.value + ports;
}

std::string element::function_declaration::to_code(int depth) const
{
    auto declaration = name.value;
    std::string ports;

    const std::string offset = "    ";
    std::string declaration_offset;

    for (auto i = 0; i < depth; ++i)
        declaration_offset += offset;

    if (has_inputs()) {
        static auto accumulate = [depth](std::string accumulator, const port& port)
        {
            return std::move(accumulator) + ", " + port.to_string() + port.to_code(depth);
        };

        const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].to_string() + inputs[0].to_code(depth), accumulate);
        ports = "(" + input_ports + ")";
    }

    if (output)
        ports += output->to_code(depth);

    if (_intrinsic)
        return declaration_offset + "intrinsic " + name.value + ports + ";";

    return declaration_offset + name.value + ports + our_scope->to_code(depth);
}

std::string element::namespace_declaration::to_code(const int depth) const
{
    return "namespace " + name.value + our_scope->to_code(depth);
}

std::string element::expression_chain::to_code(int depth) const
{
    static auto accumulate = [](std::string accumulator, const std::unique_ptr<expression>& expression)
    {
        return std::move(accumulator) + expression->to_code();
    };

    return std::accumulate(std::next(std::begin(expressions)), std::end(expressions), expressions[0]->to_code(), accumulate);
}

std::string element::call_expression::to_code(int depth) const
{
    static auto accumulate = [](std::string accumulator, const std::unique_ptr<expression_chain>& chain)
    {
        return std::move(accumulator) + ", " + chain->to_code();
    };

    const auto expressions = std::accumulate(std::next(std::begin(arguments)), std::end(arguments), arguments[0]->to_code(), accumulate);
    return "(" + expressions + ")";
}

std::string element::port::to_code(int depth) const
{
    if (annotation)
        return annotation->to_code(depth);

    return "";
}

std::string element::scope::to_code(int depth) const
{
    std::string code;

    const std::string offset = "    ";
    std::string scope_offset;

    for (auto i = 0; i < depth; ++i)
        scope_offset += offset;

    const auto end = std::end(declarations);
    for (auto declaration = std::begin(declarations); declaration != end; ++declaration)
        code += declaration->second->to_code(depth + 1) + (std::next(declaration) == end ? "" : "\n");

    if (is_root())
        return code;

    return "\n" + scope_offset + "{\n" + code + "\n" + scope_offset + "}";
}

std::string element::type_annotation::to_code(const int depth) const
{
    return ":" + name.value;
}

//std::string lambda_expression::to_string() const
//{
//    return "_";
//}
//
//std::string lambda_expression::to_code(int depth) const
//{
//    return "_";
//}
//
//std::string expression_bodied_lambda_expression::to_string() const
//{
//    return "_";
//}
//
//std::string expression_bodied_lambda_expression::to_code(int depth) const
//{
//    return "_";
//}