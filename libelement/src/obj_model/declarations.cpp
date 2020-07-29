#include "declarations.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "object.hpp"
#include "scope.hpp"
#include "expressions.hpp"
#include "intermediaries.hpp"
#include "functions.hpp"
#include "intrinsics.hpp"
#include "errors.hpp"

namespace element
{
    declaration::declaration(identifier name, const scope* parent)
        : name(std::move(name))
        , our_scope(std::make_unique<scope>(parent, this))
    {
        assert(parent);
    }

    bool declaration::has_scope() const
    {
        return !our_scope->is_empty();
    }

    std::string declaration::location() const
    {
        assert(our_scope && our_scope->get_parent_scope());

        auto& declaration = name.value;

        if (our_scope->get_parent_scope()->is_root())
            return declaration;

        //recursive construction
        return fmt::format("{}.{}", our_scope->get_parent_scope()->location(), declaration);
    }

    //struct
    struct_declaration::struct_declaration(identifier name, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = struct_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::shared_ptr<object> struct_declaration::index(
        const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const
    {
        return our_scope->find(name, false);
    }

    std::shared_ptr<object> struct_declaration::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        //this function handles construction of an intrinsic struct instance (get_intrinsic(...)->call(...)) or a user struct instance (make_shared<struct_instance>(...))
        if (is_intrinsic()) 
        {
            const auto intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
            if (intrinsic) 
                return intrinsic->call(context, compiled_args, source_info);

            //todo: could we validate this when creating the object model? then there's less to check during compilation
            return build_error_and_log(context, source_info, error_message_code::intrinsic_not_implemented);
        }

        if (valid_call(this, compiled_args))
            return std::make_shared<struct_instance>(this, compiled_args);

        return build_error_for_invalid_call(this, compiled_args);
    }

    bool struct_declaration::serializable(const compilation_context& context) const
    {
        //if it's a type (struct) and it can be deserialized (can be represented as a flat array of floats via placeholder expressions), then it can also be serialized (converted to an expression tree)
        return deserializable(context);
    }

    bool struct_declaration::deserializable(const compilation_context& context) const
    {
        //todo: cache deserializabilit

        if (inputs.empty())
        {
            assert(is_intrinsic());
            const auto intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
            assert(intrinsic);
            //todo: ask intrinsic if it's deserializable
            if (intrinsic->get_type() == type::num.get() || intrinsic->get_type() == type::boolean.get())
                return true;

            return false;
        }

        for (const auto& input : get_inputs())
        {
            //todo: we can cache all of the resolving annotation things everywhere
            const auto& type = get_scope()->find(input.get_annotation()->to_string(), true);
            if (!type->deserializable(context))
                return false;
        }

        return true;
    }

    std::shared_ptr<object> struct_declaration::generate_placeholder(const compilation_context& context, int& placeholder_index) const
    {
        if (inputs.empty())
        {
            assert(is_intrinsic());
            const auto intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
            auto expr = std::make_shared<element_expression_input>(placeholder_index);
            expr->actual_type = intrinsic->get_type();
            return expr;
        }

        std::vector<std::shared_ptr<object>> placeholder_inputs;
        for (const auto& input : get_inputs())
        {
            const auto& type = get_scope()->find(input.get_annotation()->to_string(), true);
            auto placeholder = type->generate_placeholder(context, placeholder_index);

            if (!placeholder)
            {
                assert(!"this type can't be deserialised");
                return nullptr;
            }

            placeholder_inputs.push_back(std::move(placeholder));
            placeholder_index++;
        }

        return call(context, std::move(placeholder_inputs), {});
    }

    //constraint
    constraint_declaration::constraint_declaration(identifier name, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = constraint_qualifier;
        _intrinsic = is_intrinsic;
    }

    //function
    function_declaration::function_declaration(identifier name, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = function_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::shared_ptr<object> function_declaration::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        const auto instance = std::make_shared<function_instance>(this, context.captures, source_info, compiled_args);
        return instance->compile(context, source_info);
    }

    std::shared_ptr<object> function_declaration::compile(const compilation_context& context, const source_information& source_info) const
    {
        const auto instance = std::make_shared<function_instance>(this, context.captures, source_info);
        return instance->compile(context, source_info);
    }

    //namespace
    namespace_declaration::namespace_declaration(identifier name, const scope* parent_scope)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::shared_ptr<object> namespace_declaration::index(
        const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const
    {
        return our_scope->find(name, false);
    }
}