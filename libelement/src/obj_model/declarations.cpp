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

    std::shared_ptr<object> struct_declaration::index(const compilation_context& context, const identifier& name, const source_information& source_info) const
    {
        return our_scope->find(name, false);
    }

    std::shared_ptr<object> struct_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args, const source_information& source_info) const
    {
        //this function handles construction of an intrinsic struct instance (get_intrinsic(...)->call(...)) or a user struct instance (make_shared<struct_instance>(...))
        if (is_intrinsic()) 
        {
            const auto intrinsic = intrinsic::get_intrinsic(*this);
            if (intrinsic) 
                return intrinsic->call(context, compiled_args, source_info);

            return build_error(source_info, error_message_code::is_not_an_instance_function);
        }
        
        return std::make_shared<struct_instance>(this, compiled_args);
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

    std::shared_ptr<object> function_declaration::call(const compilation_context& context, std::vector<std::shared_ptr<object>> compiled_args, const source_information&
                                                       source_info) const
    {
        const auto ret = std::make_shared<function_instance>(this, context.stack, std::vector<std::shared_ptr<object>>{});
        return ret->call(context, compiled_args, source_info);
    }

    std::shared_ptr<object> function_declaration::compile(const compilation_context& context, const source_information& source_info) const
    {
        const auto ret = std::make_shared<function_instance>(this, context.stack, std::vector<std::shared_ptr<object>>{});
        return ret->compile(context, source_info);
    }

    std::shared_ptr<object> function_declaration::is_recursive(const compilation_context& context) const
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
            if (it != context.stack.frames.rend() - 1)
                trace += "\n";
        }

        return build_error(source_info, error_message_code::recursion_detected, typeof_info(), trace);
    }

    //namespace
    namespace_declaration::namespace_declaration(identifier name, const scope* parent_scope)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::shared_ptr<object> namespace_declaration::index(const compilation_context& context, const element::identifier& name, const source_information& source_info) const
    {
        return our_scope->find(name, false);
    }
}