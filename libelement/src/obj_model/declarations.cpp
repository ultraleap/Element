#include "declarations.hpp"

//SELF
#include "object.hpp"
#include "scope.hpp"
#include "expressions.hpp"
#include "intermediaries.hpp"
#include "functions.hpp"
#include "etree/expressions.hpp"

namespace element
{
    //declaration
    declaration::declaration(identifier name)
        : declaration(std::move(name), nullptr)
    {
    }

    declaration::declaration(identifier name, const scope* parent)
        : name(std::move(name))
        , our_scope(std::make_unique<scope>(parent, this))
    {
    }

    bool declaration::has_scope() const
    {
        return our_scope && !our_scope->is_empty();
    }

    std::string declaration::location() const
    {
        auto declaration = name.value;

        if (!our_scope)
            return declaration;
        
        if (!our_scope->get_parent_scope() || our_scope->get_parent_scope()->is_root())
            return declaration;

        //recursive construction
        return our_scope->get_parent_scope()->location() + "." + declaration;
    }

    std::shared_ptr<object> declaration::compile(const compilation_context& context) const
    {
        return body->compile(context);
    }

    //struct
    struct_declaration::struct_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = struct_qualifier;
    }

    std::string struct_declaration::to_string() const
    {
        return location() + ":Struct";
    }

    std::string struct_declaration::to_code(const int depth) const
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

    std::shared_ptr<object> struct_declaration::index(const compilation_context& context, const identifier& identifier) const
    {
        return our_scope->find(identifier, false);
    }

    //constraint
    constraint_declaration::constraint_declaration(identifier identifier, const bool is_intrinsic)
        : declaration(std::move(identifier))
    {
        qualifier = constraint_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::string constraint_declaration::to_string() const
    {
        return location() + ":Constraint";
    }

    std::string constraint_declaration::to_code(const int depth) const
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

    //function
    function_declaration::function_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier)
        , parent_scope)
    {
        qualifier = function_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::string function_declaration::to_string() const
    {
        return location() + ":Function";
    }

    std::string function_declaration::to_code(int depth) const
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

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> args) const
    {
        return body->call(context, args);
    }

    //namespace
    namespace_declaration::namespace_declaration(identifier identifier, const scope* parent_scope)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::string namespace_declaration::to_string() const
    {
        return location() + ":Namespace";
    }

    std::string namespace_declaration::to_code(const int depth) const
    {
        return "namespace " + name.value + our_scope->to_code(depth);
    }

    std::shared_ptr<object> namespace_declaration::index(const compilation_context& context, const element::identifier& identifier) const
    {
        return our_scope->find(identifier, false);
    }
}
