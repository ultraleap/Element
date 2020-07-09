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

        //TODO: If get_intrinsic returns null return error ELEMENT_ERROR_INTRINSIC_NOT_IMPLEMENTED
        if (_intrinsic)
            body = intrinsic::get_intrinsic(*this);
        else
            body = std::make_shared<intrinsic_user_constructor>(this);
    }

    std::shared_ptr<object> struct_declaration::index(const compilation_context& context, const identifier& name) const
    {
        return our_scope->find(name, false);
    }

    std::shared_ptr<object> struct_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        //todo: add frame instead of using arguments?
        return body->call(context, compiled_args);
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

    std::shared_ptr<object> function_declaration::index(const compilation_context& context, const identifier& name) const
    {
        return call(context, {})->index(context, name);
    }

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args) const
    {
        //todo: this and compile are almost identical

        if (context.is_recursive(this))
        {
            std::string trace;
            //todo: have it on the stack?
            for (auto it = context.stack.frames.rbegin(); it < context.stack.frames.rend(); ++it)
            {
                auto& func = it->function;
                std::string params;
                for (unsigned i = 0; i < func->inputs.size(); ++i)
                {
                    const auto& input = func->inputs[i];
                    params += fmt::format("{}{} = {}",
                        input.name.value, input.annotation ? ":" + input.annotation->name.value : "", it->compiled_arguments[i]->typeof_info())
                    ;
                    if (i != func->inputs.size() - 1)
                        params += ", ";
                }
                trace += fmt::format("{}:{} at {}({})",
                    func->source_info.filename, func->source_info.line, func->typeof_info(), params);
                if (func == this)
                    trace += " <-- here";
                trace += "\n";
            }

            return build_error(source_info, error_message_code::recursion_detected, typeof_info(), trace);
        }

        call_stack::frame frame;
        frame.function = this;
        frame.compiled_arguments = std::move(compiled_args);
        context.stack.frames.emplace_back(std::move(frame));

        if (!body)
        {
            return std::make_shared<error>(
                fmt::format("failed at {}. scope bodied functions must contain a return function.\n", typeof_info()),
                ELEMENT_ERROR_MISSING_FUNCTION_BODY,
                source_info);
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
                std::string trace;
                //todo: have it on the stack?
                for (auto it = context.stack.frames.rbegin(); it < context.stack.frames.rend(); ++it)
                {
                    auto& func = it->function;
                    std::string params;
                    for (unsigned i = 0; i < func->inputs.size(); ++i)
                    {
                        const auto& input = func->inputs[i];
                        //todo: instead of typeof info for the compiled arg, see if we can evaluate it and print the value
                        params += fmt::format("{}{} = {}",
                            input.name.value, input.annotation ? ":" + input.annotation->name.value : "", it->compiled_arguments[i]->typeof_info());
                        if (i != func->inputs.size() - 1)
                            params += ", ";
                    }
                    trace += fmt::format("{}:{} at {}({})",
                        func->source_info.filename, func->source_info.line, func->typeof_info(), params);
                    if (func == this)
                        trace += " <-- here";
                    trace += "\n";
                }

                return build_error(source_info, error_message_code::recursion_detected, typeof_info(), trace);
            }

            call_stack::frame frame;
            frame.function = this;
            context.stack.frames.emplace_back(std::move(frame));

            if (!body)
            {
                return std::make_shared<error>(
                    fmt::format("failed at {}. scope bodied functions must contain a return function.\n", typeof_info()),
                    ELEMENT_ERROR_MISSING_FUNCTION_BODY,
                    source_info);
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
    namespace_declaration::namespace_declaration(identifier name, const scope* parent_scope)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::shared_ptr<object> namespace_declaration::index(const compilation_context& context, const element::identifier& name) const
    {
        return our_scope->find(name, false);
    }
}