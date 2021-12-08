#include "metainfo.hpp"
#include "declarations/declaration.hpp"
#include "declarations/constraint_declaration.hpp"
#include "declarations/struct_declaration.hpp"
#include "declarations/function_declaration.hpp"
#include "declarations/namespace_declaration.hpp"
#include "expressions/expression.hpp"
#include "expressions/expression_chain.hpp"
#include "expressions/call_expression.hpp"
#include "expressions/identifier_expression.hpp"
#include "expressions/indexing_expression.hpp"
#include "intermediaries/struct_instance.hpp"
#include "intermediaries/function_instance.hpp"
#include "intrinsics/intrinsic.hpp"
#include "constraints/constraint.hpp"
#include "constraints/type.hpp"
#include "port.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "instruction_tree/instructions.hpp"
#include "expressions/anonymous_block_expression.hpp"

#include <fmt/format.h>
#include <sstream>

using namespace element;

//to_string
std::string constraint::typeof_info() const
{
    return declarer->location() + ":Constraint";
}

std::string struct_declaration::typeof_info() const
{
    return is_intrinsic() ? "IntrinsicStruct" : "CustomStruct";
}

std::string constraint_declaration::typeof_info() const
{
    return is_intrinsic() ? "IntrinsicConstraint" : "FunctionConstraint"; //Might have more constraints later... derived types could handle this function return value instead
}

std::string function_declaration::typeof_info() const
{
    switch (function_kind) {
    case kind::intrinsic:
        return "IntrinsicFunction";
    case kind::scope_bodied:
        return "ScopeBodiedFunction";
    case kind::expression_bodied:
        return "ExpressionBodiedFunction";
    }
    return {};
}

std::string namespace_declaration::typeof_info() const
{
    return "Namespace";
}

std::string expression_chain::get_name() const
{
    return "";
}

std::string expression_chain::typeof_info() const
{
    //TODO: We need an override here since object::typeof_info is pure virtual, but is a value required here?
    return declarer->typeof_info() + ":ExpressionChain";
}

std::string struct_instance::typeof_info() const
{
    return declarer->name.value;
}

std::string function_instance::typeof_info() const
{
    return provided_arguments.empty() ? declarer->typeof_info() : "AppliedFunction";
}

std::string intrinsic::typeof_info() const
{
    //TODO: We need an override here since object::typeof_info is pure virtual, but is a value required here?
    return "";
}

//std::string type::typeof_info() const
//{
//    //TODO: We need an override here since object::typeof_info is pure virtual, but is a value required here?
//    return declarer->location() + ":Type";
//}
//
//std::string error::typeof_info() const
//{
//    //TODO: We need an override here since object::typeof_info is pure virtual, but is a value required here?
//    return "Error: " + message;
//}

std::string instruction::typeof_info() const
{
    return actual_type ? actual_type->get_identifier().value : "UnknownType:Instruction";
}

std::string port::typeof_info() const
{
    if (!has_annotation() || annotation->to_string().empty()) {
        return name.value;
    }

    return name.value + ":" + annotation->to_string();
}

//to_code
std::string constraint::to_code(const int depth) const
{
    return "?";
}

std::string constraint::get_name() const
{
    if (declarer)
        return declarer->get_name();

    return "";
}

std::string struct_declaration::to_code(const int depth) const
{
    std::string ports;

    const std::string offset = "    ";
    std::string result;

    for (auto i = 0; i < depth; ++i)
        result += offset;

    if (has_inputs()) {
        static auto accumulate = [depth](std::string accumulator, const port& port) {
            return std::move(accumulator) + ", " + port.typeof_info() + port.to_code(depth);
        };

        const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].typeof_info() + inputs[0].to_code(depth), accumulate);
        ports = "(" + input_ports + ")";
    }

    result += is_intrinsic() ? "intrinsic struct " : "struct ";
    result += name.value + ports;
    result += has_scope() ? our_scope->to_code(depth) : "";
    return result;
}

std::string constraint_declaration::to_code(const int depth) const
{
    std::string ports;

    const std::string offset = "    ";
    std::string result;

    for (auto i = 0; i < depth; ++i)
        result += offset;

    if (has_inputs()) {
        static auto accumulate = [depth](std::string accumulator, const port& port) {
            return std::move(accumulator) + ", " + port.typeof_info() + port.to_code(depth);
        };

        const auto input_ports = std::accumulate(std::next(std::begin(inputs)), std::end(inputs), inputs[0].typeof_info() + inputs[0].to_code(depth), accumulate);
        ports = "(" + input_ports + ")";
    }

    ports += output.to_code(depth);

    result += is_intrinsic() ? "intrinsic constraint " : "constraint ";
    result += name.value + ports;
    return result;
}

std::string function_declaration::to_code(const int depth) const
{
    auto declaration = name.value;

    const std::string offset = "    ";
    std::string result;

    for (auto i = 0; i < depth; ++i)
        result += offset;

    std::string ports;
    if (has_inputs()) {
        static constexpr auto accumulate = [](std::string accumulator, const port& port) {
            return std::move(accumulator) + ", " + port.typeof_info();
        };

        const auto input_ports = std::accumulate(
            std::next(std::begin(inputs)),
            std::end(inputs),
            inputs[0].typeof_info(),
            accumulate);

        ports = "(" + input_ports + ")";
    }

    ports += output.to_code(depth);

    //intrinsic declaration
    if (is_intrinsic())
        return result + "intrinsic " + name.value + ports;

    //scope-bodied
    if (has_scope())
        return result + name.value + ports + our_scope->to_code(depth);

    const auto visitor = [depth](auto& body) {
        return body->to_code(depth);
    };

    //expression-bodied
    return result + name.value + ports + " = " + std::visit(visitor, body);
}

std::string function_declaration::to_string() const
{
    return to_code(0);
}

std::string namespace_declaration::to_code(const int depth) const
{
    return "namespace " + name.value + our_scope->to_code(depth);
}

std::string expression_chain::to_code(const int depth) const
{
    static auto accumulate = [](std::string accumulator, const std::unique_ptr<expression>& expression) {
        return std::move(accumulator) + expression->to_code();
    };

    return std::accumulate(std::next(std::begin(expressions)), std::end(expressions), expressions[0]->to_code(), accumulate);
}

std::string identifier_expression::to_code(const int depth) const
{
    return name.value;
}

std::string call_expression::to_code(const int depth) const
{
    static auto accumulate = [](std::string accumulator, const std::unique_ptr<expression_chain>& chain) {
        return std::move(accumulator) + ", " + chain->to_code();
    };

    const auto expressions = std::accumulate(std::next(std::begin(arguments)), std::end(arguments), arguments[0]->to_code(), accumulate);
    return "(" + expressions + ")";
}

std::string indexing_expression::to_code(const int depth) const
{
    return "." + name.value;
}

std::string anonymous_block_expression::to_code(const int depth) const
{
    return "<TODO>";
}

std::string port::to_code(const int depth) const
{
    if (has_annotation())
        return annotation->to_code(depth);

    return "";
}

std::string scope::to_code(const int depth) const
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

std::string type_annotation::to_code(const int depth) const
{
    if (name.value.empty())
        return {};

    return ":" + name.value;
}

// instructions

std::string instruction_constant::to_code(const int depth) const
{
    if (actual_type == type::boolean.get())
        return to_bool(m_value) ? "true" : "false";

    return fmt::format("{:g}", m_value);
}

std::string instruction_input::to_code(const int depth) const
{
    // TODO: ???
    return to_string();
}

std::string instruction_serialised_structure::to_code(const int depth) const
{
    std::stringstream ss;
    ss << m_debug_type_name;
    if (!m_dependents.empty()) {
        ss << "(";
        for (size_t i = 0; i < m_dependents.size(); ++i) {
            if (i) ss << ",";
            ss << m_dependents[i]->to_code(depth);
        }
        ss << ")";
    }
    return ss.str();
}

// TODO: not have all these symbol names hard-coded in here :(
std::string instruction_nullary::to_code(const int depth) const
{
    switch (operation()) {
        case op::true_value: return "true";
        case op::false_value: return "false";
        case op::nan: return "NaN";
        case op::positive_infinity: return "PositiveInfinity";
        case op::negative_infinity: return "NegativeInfinity";
        default: return "<INVALID>";
    }
}

// TODO: not have all these symbol names hard-coded in here :(
std::string instruction_unary::to_code(const int depth) const
{
    std::string opname;
    switch (operation()) {
        case op::abs: opname = "abs"; break;
        case op::acos: opname = "acos"; break;
        case op::asin: opname = "asin"; break;
        case op::atan: opname = "atan"; break;
        case op::ceil: opname = "ceil"; break;
        case op::cos: opname = "cos"; break;
        case op::floor: opname = "floor"; break;
        case op::ln: opname = "ln"; break;
        case op::not_: opname = "not"; break;
        case op::sin: opname = "sin"; break;
        case op::tan: opname = "tan"; break;
        default: return "<INVALID>";
    }

    return fmt::format("{}({})", opname, input()->to_code(depth));
}

// TODO: not have all these symbol names hard-coded in here :(
std::string instruction_binary::to_code(const int depth) const
{
    std::string opname;
    switch (operation()) {
        case op::add: opname = "add"; break;
        case op::sub: opname = "sub"; break;
        case op::mul: opname = "mul"; break;
        case op::div: opname = "div"; break;
        case op::rem: opname = "rem"; break;
        case op::pow: opname = "pow"; break;
        case op::min: opname = "min"; break;
        case op::max: opname = "max"; break;
        case op::log: opname = "log"; break;
        case op::atan2: opname = "atan2"; break;
        case op::and_: opname = "and"; break;
        case op::or_: opname = "or"; break;
        case op::eq: opname = "eq"; break;
        case op::neq: opname = "neq"; break;
        case op::lt: opname = "lt"; break;
        case op::leq: opname = "leq"; break;
        case op::gt: opname = "gt"; break;
        case op::geq: opname = "geq"; break;
        default: return "<INVALID>";
    }

    return fmt::format("{}({},{})", opname, input1()->to_code(depth), input2()->to_code(depth));
}

std::string instruction_if::to_code(const int depth) const
{
    return fmt::format("if({},{},{})",
        predicate()->to_code(depth),
        if_true()->to_code(depth),
        if_false()->to_code(depth));
}

std::string instruction_for::to_code(const int depth) const
{
    return fmt::format("for({},{},{})",
        initial()->to_code(depth),
        condition()->to_code(depth),
        body()->to_code(depth));
}

std::string instruction_indexer::to_code(const int depth) const
{
    // TODO: ???
    return "<TODO>";
}

std::string instruction_select::to_code(const int depth) const
{
    // TODO: ???
    return "<TODO>";
}
