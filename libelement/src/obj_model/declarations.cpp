#include "declarations.hpp"

//LIBS
#include <fmt/format.h>

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

    //struct
    struct_declaration::struct_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = struct_qualifier;
        _intrinsic = is_intrinsic;

        if (_intrinsic)
            body = intrinsic::get_intrinsic(*this);
        else
            body = std::make_shared<intrinsic_user_constructor>(this);
    }

    std::shared_ptr<object> struct_declaration::index(const compilation_context& context, const identifier& identifier) const
    {
        return our_scope->find(identifier, false);
    }

    std::shared_ptr<object> struct_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        //todo: add frame instead of using arguments?
        return body->call(context, compiled_args);
    }

    //constraint
    constraint_declaration::constraint_declaration(identifier identifier, const bool is_intrinsic)
        : declaration(std::move(identifier))
    {
        qualifier = constraint_qualifier;
        _intrinsic = is_intrinsic;
    }

    //function
    function_declaration::function_declaration(identifier identifier, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(identifier)
        , parent_scope)
    {
        qualifier = function_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::shared_ptr<object> function_declaration::index(const compilation_context& context, const identifier& name) const
    {
        return call(context, {})->index(context, name);
    }

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        //todo: this and compile are almost identical

        if (context.is_recursive(this))
        {
            return std::make_shared<error>(
                fmt::format("failed at {}. recursion detected.\n", to_string()),
                ELEMENT_ERROR_CIRCULAR_COMPILATION,
                this);
        }

        call_stack::frame frame;
        frame.function = this;
        frame.compiled_arguments = std::move(compiled_args);
        context.stack.frames.emplace_back(std::move(frame));

        if (!body)
        {
            return std::make_shared<error>(
                fmt::format("failed at {}. scope bodied functions must contain a return function.\n", to_string()),
                ELEMENT_ERROR_MISSING_FUNCTION_BODY,
                this);
        }

        //todo: we don't need args passed since it's in the stack
        auto ret = body->call(context, context.stack.frames.back().compiled_arguments);

        context.stack.frames.pop_back();
        return ret;
    }

    std::shared_ptr<object> function_declaration::compile(const compilation_context& context) const
    {
        std::shared_ptr<object> ret;

        if (inputs.empty())
        {
            //Nullary, compile the body
            if (context.is_recursive(this))
            {
                return std::make_shared<error>(
                    fmt::format("failed at {}. recursion detected.\n", to_string()),
                    ELEMENT_ERROR_CIRCULAR_COMPILATION,
                    this);
            }

            call_stack::frame frame;
            frame.function = this;
            context.stack.frames.emplace_back(std::move(frame));

            if (!body)
            {
                return std::make_shared<error>(
                    fmt::format("failed at {}. scope bodied functions must contain a return function.\n", to_string()),
                    ELEMENT_ERROR_MISSING_FUNCTION_BODY,
                    this);
            }

            ret = body->compile(context);

            context.stack.frames.pop_back();
        }
        else
        {
            //Not nullary, create function instance for higher order function stuff
            //We don't create a new frame here since when the function instance is called, it'll call the declaration which will create the frame
            ret = std::make_shared<function_instance>(this, context.stack, std::vector<std::shared_ptr<object>>{});
        }

        return ret;
    }

    //namespace
    namespace_declaration::namespace_declaration(identifier identifier, const scope* parent_scope)
        : declaration(std::move(identifier), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::shared_ptr<object> namespace_declaration::index(const compilation_context& context, const element::identifier& identifier) const
    {
        return our_scope->find(identifier, false);
    }
}