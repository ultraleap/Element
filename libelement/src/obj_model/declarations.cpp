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
            const auto intrinsic = intrinsic::get_intrinsic(*this);
            if (intrinsic) 
                return intrinsic->call(context, compiled_args, source_info);

            //todo: could we validate this when creating the object model? then there's less to check during compilation
            return build_error(source_info, error_message_code::intrinsic_not_implemented);
        }

        if (valid_call(this, compiled_args))
            return std::make_shared<struct_instance>(this, compiled_args);

        return build_error_for_invalid_call(this, compiled_args);
    }

    //constraint
    constraint_declaration::constraint_declaration(identifier name, const bool is_intrinsic)
        : declaration(std::move(name))
    {
        qualifier = constraint_qualifier;
        _intrinsic = is_intrinsic;
    }

    //function
    function_declaration::function_declaration(identifier name, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(name)
        , parent_scope)
    {
        qualifier = function_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::shared_ptr<object> function_declaration::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        //todo: could we validate this when creating the object model? then there's less to check during compilation
        if (!body)
        {
            return std::make_shared<error>(
                fmt::format("failed at {}. scope bodied functions must contain a return function.\n", typeof_info()),
                ELEMENT_ERROR_MISSING_FUNCTION_BODY,
                source_info);
        }

        auto captures = capture_stack(this, context.calls);
        const auto ret = std::make_shared<function_instance>(this, context.calls, std::move(captures), compiled_args);
        ret->source_info = source_info;
        return ret->compile(context, source_info);
    }

    std::shared_ptr<object> function_declaration::compile(const compilation_context& context, const source_information& source_info) const
    {
        return call(context, {}, source_info);
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