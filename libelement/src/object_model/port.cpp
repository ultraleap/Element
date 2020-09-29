#include "port.hpp"

//LIB
#include <fmt/format.h>

//SELF
#include "declarations/declaration.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "compilation_context.hpp"

using namespace element;

port::port(const declaration* declarer, identifier name, std::unique_ptr<type_annotation> annotation)
    : declarer(declarer)
    , name{ std::move(name) }
    , annotation{ std::move(annotation) }
{
}

const declaration* port::resolve_annotation(const compilation_context& context) const
{
    if (!annotation)
        return context.get_global_scope()->find(identifier{ "Any" }, false);

    const auto* decl = declarer->get_scope()->find(annotation->to_string(), true);
    if (const auto* func_decl = dynamic_cast<const function_declaration*>(decl))
    {
        error(fmt::format("'{}' is not a constraint, but a function", annotation->to_string()), ELEMENT_ERROR_NOT_A_CONSTRAINT, declarer->source_info).log_once(context.get_logger());
        return nullptr;
    }

    return decl;
}

object_const_shared_ptr port::generate_placeholder(const compilation_context& context, int& placeholder_index) const
{
    const auto* type = resolve_annotation(context);
    if (!type)
        return nullptr;

    return type->generate_placeholder(context, placeholder_index);
}