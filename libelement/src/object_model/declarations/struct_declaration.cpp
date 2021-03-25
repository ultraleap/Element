#include "struct_declaration.hpp"

//SELF
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/intrinsics/intrinsic.hpp"
#include "object_model/scope.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/intermediaries/struct_instance.hpp"
#include "object_model/constraints/user_type.hpp"
#include "instruction_tree/instructions.hpp"

using namespace element;

struct_declaration::struct_declaration(identifier name, const scope* parent_scope, const kind struct_kind)
    : declaration(name, parent_scope)
    , type(std::make_unique<user_type>(std::move(name), this))
    , struct_kind(struct_kind)
{
    qualifier = struct_qualifier;
}

object_const_shared_ptr struct_declaration::index(
    const compilation_context& context,
    const identifier& name,
    const source_information& source_info) const
{
    if (our_scope->is_empty())
        return std::make_shared<const error>(
            "Structs with empty scopes cannot be indexed",
            ELEMENT_ERROR_INVALID_EXPRESSION,
            source_info,
            context.get_logger());

    const auto* found = our_scope->find(name, context.interpreter->caches, false);
    if (!found)
        return build_error_and_log(context, source_info, error_message_code::failed_to_find_when_resolving_indexing_expr, name.value, to_string());

    return found->compile(context, source_info);
}

object_const_shared_ptr struct_declaration::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    //this function handles construction of an intrinsic struct instance (get_intrinsic(...)->call(...)) or a user struct instance (make_shared<struct_instance>(...))
    if (is_intrinsic())
    {
        const auto* intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
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
        const auto* const intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
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
    //if it's a type (struct) and it can be deserialized (can be represented as a flat array of floats via placeholder instructions), then it can also be serialized (converted to an instruction tree)
    return deserializable(context);
}

bool struct_declaration::deserializable(const compilation_context& context) const
{
    //todo: cache deserializabilit

    if (inputs.empty())
    {
        assert(is_intrinsic());
        const auto* intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
        assert(intrinsic);
        //todo: ask intrinsic if it's deserializable
        if (intrinsic->get_type() == type::num.get() || intrinsic->get_type() == type::boolean.get())
            return true;

        return false;
    }

    for (const auto& input : get_inputs())
    {
        //todo: we can cache all of the resolving annotation things everywhere
        const auto& type = get_scope()->find(input.get_annotation()->to_string(), context.interpreter->caches, true);
        assert(type);
        if (!type->deserializable(context))
            return false;
    }

    return true;
}

object_const_shared_ptr struct_declaration::generate_placeholder(const compilation_context& context, int& placeholder_index, unsigned int boundary_scope) const
{
    if (inputs.empty())
    {
        assert(is_intrinsic());
        const auto* intrinsic = intrinsic::get_intrinsic(context.interpreter, *this);
        auto expr = std::make_shared<instruction_input>(boundary_scope, placeholder_index);
        expr->actual_type = intrinsic->get_type();
        placeholder_index += 1; //todo: fix when we have lists, size() on intrinsic? on type?
        return expr;
    }

    std::vector<object_const_shared_ptr> placeholder_inputs;
    for (const auto& input : get_inputs())
    {
        const auto& type = get_scope()->find(input.get_annotation()->to_string(), context.interpreter->caches, true);
        auto placeholder = type->generate_placeholder(context, placeholder_index, boundary_scope);

        if (!placeholder)
        {
            return std::make_shared<const error>(
                fmt::format("The type '{}' can't be deserialised.", to_string()),
                ELEMENT_ERROR_SERIALISATION,
                source_info,
                context.get_logger());
        }

        placeholder_inputs.push_back(std::move(placeholder));
    }

    return call(context, std::move(placeholder_inputs), {});
}

bool struct_declaration::is_intrinsic() const
{
    return struct_kind == kind::intrinsic;
}