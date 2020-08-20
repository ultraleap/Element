#include "declarations.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "object.hpp"
#include "scope.hpp"
#include "expressions.hpp"
#include "intermediaries.hpp"
#include "intrinsics.hpp"
#include "errors.hpp"
#include "etree/expressions.hpp"

namespace element
{
    declaration::declaration(identifier name, const scope* parent)
        : name(std::move(name))
        , our_scope(std::make_unique<scope>(parent, this))
        , wrapper(std::make_shared<declaration_compilation_wrapper>(this))
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
        : declaration(name, parent_scope)
        , type(std::make_unique<user_type>(std::move(name), this))
    {
        qualifier = struct_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::shared_ptr<const object> struct_declaration::index(
        const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const
    {
        const auto* found = our_scope->find(name, false);
        if (!found)
            return build_error_and_log(context, source_info, error_message_code::failed_to_find_when_resolving_indexing_expr, name.value, typeof_info());

        return found->compile(context, source_info);
    }

    std::shared_ptr<const object> struct_declaration::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<const object>> compiled_args,
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

        if (valid_call(context, this, compiled_args))
            return std::make_shared<struct_instance>(this, compiled_args);

        return build_error_for_invalid_call(context, this, compiled_args);
    }

    bool struct_declaration::matches_constraint(const compilation_context& context, const constraint* constraint) const
    {
        if (is_intrinsic())
        {
            const auto intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
            if (!intrinsic)
                return type->matches_constraint(context, constraint);

            return intrinsic->matches_constraint(context, constraint);
        }

        return type->matches_constraint(context, constraint);
    }

    const constraint* struct_declaration::get_constraint() const
    {
        //todo: need context to grab intrinsic
        if (name.value == "Num")
            return type::num.get();

        if (name.value == "Bool")
            return type::boolean.get();

        return type.get();
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

    std::shared_ptr<const object> struct_declaration::generate_placeholder(
        const compilation_context& context, int& placeholder_index) const
    {
        if (inputs.empty())
        {
            assert(is_intrinsic());
            const auto intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
            auto expr = std::make_shared<element_expression_input>(placeholder_index);
            expr->actual_type = intrinsic->get_type();
            return expr;
        }

        std::vector<std::shared_ptr<const object>> placeholder_inputs;
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
        : declaration(name, parent_scope)
        , constraint_(std::make_unique<constraint>(4, this)) //todo: what to use
    {
        qualifier = constraint_qualifier;
        _intrinsic = is_intrinsic;
    }

    bool constraint_declaration::matches_constraint(const compilation_context& context, const constraint* constraint) const
    {
        return constraint_->matches_constraint(context, constraint);
    }

    const constraint* constraint_declaration::get_constraint() const
    {
        if (is_intrinsic())
            return constraint::any.get();

        return constraint_.get();
    }

    /*   
    constraint func_constraint(a:Num):Num;
    do_nothing(a:Num):Num = a;
    do_more_nothing(a:Num):Num = a;

    call_func(func:func_constraint, b:Num) = func(b);
    
    evaluate = call_func(do_nothing, 1); //returns 1
    evaluate = call_func(do_more_nothing, 1); //returns 1
    */


    //function
    function_declaration::function_declaration(identifier name, const scope* parent_scope, const bool is_intrinsic)
        : declaration(std::move(name), parent_scope)
        , constraint_ (std::make_unique<user_function_constraint>(this)) //todo: what to use
    {
        qualifier = function_qualifier;
        _intrinsic = is_intrinsic;
    }

    std::shared_ptr<const object> function_declaration::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<const object>> compiled_args,
        const source_information& source_info) const
    {
        //todo: check, if there is a first argument, that this is a valid instance function
        //todo: check that there is only one argument
        const auto instance = std::make_shared<function_instance>(this, context.captures, source_info, compiled_args);
        return instance->compile(context, source_info);
    }

    std::shared_ptr<const object> function_declaration::compile(const compilation_context& context,
                                                                const source_information& source_info) const
    {
        const auto instance = std::make_shared<function_instance>(this, context.captures, source_info);
        return instance->compile(context, source_info);
    }

    bool function_declaration::matches_constraint(const compilation_context& context, const constraint* constraint) const
    {
        return constraint_->matches_constraint(context, constraint);
    }

    const constraint* function_declaration::get_constraint() const
    {
        return constraint_.get();
    }

    bool function_declaration::valid_at_boundary(const compilation_context& context) const
    {
        if (!output)
            return false;

        //outputs must be serializable
        const auto return_type = output->resolve_annotation(context);
        if (!return_type || !return_type->serializable(context))
            return false;

        //inputs must be deserializable
        for (const auto& input : inputs)
        {
            const auto& type = input.resolve_annotation(context);
            if (!type || !type->deserializable(context))
                return false;
        }

        return true;
    }

    //namespace
    namespace_declaration::namespace_declaration(identifier name, const scope* parent_scope)
        : declaration(std::move(name), parent_scope)
    {
        qualifier = namespace_qualifier;
        _intrinsic = false;
    }

    std::shared_ptr<const object> namespace_declaration::index(
        const compilation_context& context,
        const identifier& name,
        const source_information& source_info) const
    {
        return our_scope->find(name, false)->compile(context, source_info);
    }
}