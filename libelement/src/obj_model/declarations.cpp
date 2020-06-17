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
        return our_scope->find(identifier.value, false);
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

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        if (inputs.size() == args.size())
            return body->call(context, args);

        //don't have all the arguments, so partially apply them
        //is a compiled expression because if the call is the full expression, then we return it from expression->compile, which needs to be a compiled expression.
        //todo: need to figure out differences between compiling and calling and their restrictions, if any
        auto result = std::make_shared<compiled_expression>();
        result->creator = this;
        result->object_model = std::make_shared<function_instance>(this, std::move(args));
        return std::move(result);
    }

    std::shared_ptr<compiled_expression> function_declaration::compile(const compilation_context& context) const
    {
        if (inputs.empty())
        {
            auto obj = call(context, {});

            if (dynamic_cast<compiled_expression*>(obj.get()))
                return std::dynamic_pointer_cast<compiled_expression>(obj);

            auto compiled = std::make_shared<compiled_expression>();
            compiled->creator = this;
            compiled->object_model = std::move(obj);
            return compiled;
        }

        throw;
    }

    //expression bodied function
    expression_bodied_function_declaration::expression_bodied_function_declaration(identifier identifier, const scope* parent_scope)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = function_qualifier;
        _intrinsic = false;
    }

    std::string expression_bodied_function_declaration::to_string() const
    {
        return location() + ":Function";
    }

    std::string expression_bodied_function_declaration::to_code(const int depth) const
    {
        auto declaration = name.value;

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
            declaration = name.value + "(" + input_ports + ")";
        }

        if (output)
            declaration += output->to_code(depth);

        return declaration_offset + declaration + " = " + expression->to_code() + ";";
    }

    std::shared_ptr<object> expression_bodied_function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const
    {
        //todo: apply arguments to callstack/cache so they can be found from scope lookups
        const auto& compiled = compile(context);
        //if it was a function instance then we should apply the arguments we have to it
        //e.g. evaluate = add5(2); will call function declaration add5 with 2, so we need to apply 2 here. maybe there's a better way
        if (dynamic_cast<function_instance*>(compiled->object_model.get()) &&
            static_cast<int>(args.size()) > static_cast<int>(inputs.size())) //we were given more args than we accept, so apply the remaining ones to the function instance
        {
            const auto& instance = static_cast<function_instance*>(compiled->object_model.get());
            instance->provided_arguments.insert(instance->provided_arguments.end(), args.begin() + inputs.size(), args.end());

            //egh.. it feels like everything needs to return a compiled_expression
            if (instance->provided_arguments.size() == instance->declarer->inputs.size())
                return instance->call(context, {});
            else
                compiled->object_model = instance->call(context, {});
        }

        return compiled;
    }

    std::shared_ptr<compiled_expression> expression_bodied_function_declaration::compile(const compilation_context& context) const
    {
        //todo: should compilation have to happen via a call? I think so, so this function becomes redundant/an issue? mmmm
        return expression->compile(context);
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

    std::shared_ptr<object> namespace_declaration::index(const compilation_context& context, const element::identifier& expr) const
    {
        return our_scope->find(name.value, false);
    }
}
