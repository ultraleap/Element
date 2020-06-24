#include "declarations.hpp"

//SELF
#include "object.hpp"
#include "scope.hpp"
#include "expressions.hpp"
#include "intermediaries.hpp"
#include "functions.hpp"
#include "etree/expressions.hpp"
#include "intrinsics.hpp"

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

    std::shared_ptr<element_expression> declaration::compile(const compilation_context& context) const
    {
        return body->compile(context);
    }

    //struct
    struct_declaration::struct_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = struct_qualifier;
        _intrinsic = is_intrinsic;
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

    std::shared_ptr<object> struct_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<element_expression>> args) const
    {
        //obviously not a good thing
        if (_intrinsic)
        {
            if (name.value == "Num")
            {
                args[0]->actual_type = type::num;
                return args[0];
            }
            else if (name.value == "Bool")
            {
                auto& true_decl = *context.get_global_scope()->find(identifier("True"), false);
                auto& false_decl = *context.get_global_scope()->find(identifier("False"), false);

                auto true_expr = intrinsic::get_intrinsic(true_decl)->call(context, {});
                auto false_expr = intrinsic::get_intrinsic(false_decl)->call(context, {});

                auto ret = std::make_shared<element_expression_if>(
                    args[0],
                    std::dynamic_pointer_cast<element_expression>(true_expr),
                    std::dynamic_pointer_cast<element_expression>(false_expr));
                
                return ret;
            }
        }
        return std::shared_ptr<object>();
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

    std::shared_ptr<object> function_declaration::index(const compilation_context& context, const identifier& name) const
    {
        return call(context, {})->index(context, name);
    }

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<element_expression>> args) const
    {
        call_stack::frame frame;
        frame.function = this;
        frame.arguments = std::move(args);
        context.stack.frames.emplace_back(std::move(frame));
        auto ret = body->call(context, context.stack.frames.back().arguments); //todo: we don't need args passed as well as in the stack
        context.stack.frames.pop_back();
        return ret;
    }

    std::shared_ptr<element_expression> function_declaration::compile(const compilation_context& context) const
    {
        if (inputs.empty())
        {
            call_stack::frame frame;
            frame.function = this;
            context.stack.frames.emplace_back(std::move(frame));
            auto ret = body->compile(context); //todo: we don't need args passed as well as in the stack
            context.stack.frames.pop_back();
            return ret;
        }

        //todo: no way to compile to an intermediary (e.g. higher order function, struct instance)
        assert(false);
        return nullptr;
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
