#include "function_declaration.hpp"

//SELF
#include "object_model/constraints/user_function_constraint.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/intrinsics/intrinsic_function.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/error.hpp"

using namespace element;

function_declaration::function_declaration(identifier name, const scope* parent_scope, const kind function_kind)
    : declaration(std::move(name), parent_scope)
    , constraint_(std::make_unique<user_function_constraint>(this)) //todo: what to use
    , function_kind(function_kind)
{
    qualifier = function_qualifier;
}

bool function_declaration::is_variadic() const
{
    constexpr auto visitor = [](const auto& body) {
        const auto* body_intrinsic = dynamic_cast<const intrinsic_function*>(&*body);
        return body_intrinsic && body_intrinsic->is_variadic();
    };

    return std::visit(visitor, body);
}

object_const_shared_ptr function_declaration::call(
    const compilation_context& context,
    std::vector<object_const_shared_ptr> compiled_args,
    const source_information& source_info) const
{
    if (!ports_validated)
        validate_ports(context);

    if (!valid_ports)
        return std::make_shared<const error>(fmt::format("One or more ports on '{}' could not be found", to_string()), ELEMENT_ERROR_IDENTIFIER_NOT_FOUND, this->source_info);

    //todo: check, if there is a first argument, that this is a valid instance function
    //todo: check that there is only one argument
    const auto instance = std::make_shared<function_instance>(this, context.captures, source_info, compiled_args);
    return instance->compile(context, source_info);
}

object_const_shared_ptr function_declaration::compile(const compilation_context& context,
                                                      const source_information& source_info) const
{
    if (!ports_validated)
        validate_ports(context);

    if (!valid_ports)
        return std::make_shared<const error>(fmt::format("One or more ports on '{}' could not be found", to_string()), ELEMENT_ERROR_IDENTIFIER_NOT_FOUND, this->source_info);

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

    if (!ports_validated)
        validate_ports(context);

    if (!valid_ports)
        return false;

    //outputs must be serializable
    const auto* return_type = output->resolve_annotation(context);
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

bool function_declaration::is_intrinsic() const
{
    return function_kind == kind::intrinsic;
}

void function_declaration::validate_ports(const compilation_context& context) const
{
    ports_validated = true;
    valid_ports = true;

    for (const auto& port : inputs)
    {
        if (!port.is_valid(context))
            valid_ports = false;
    }

    if (output && output->has_annotation() && !output->is_valid(context))
        valid_ports = false;
}
