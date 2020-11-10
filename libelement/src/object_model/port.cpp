#include "port.hpp"

//LIB
#include <fmt/format.h>

//SELF
#include "declarations/declaration.hpp"
#include "declarations/struct_declaration.hpp"
#include "declarations/constraint_declaration.hpp"
#include "expressions/expression_chain.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "compilation_context.hpp"

using namespace element;

port::port()
    : port(nullptr, identifier{ "" }, std::make_unique<type_annotation>(identifier{""}), nullptr)
{
}

port::port(const declaration* declarer, identifier name, std::unique_ptr<type_annotation> annotation, std::unique_ptr<element::expression_chain> expr_chain)
    : declarer(declarer)
    , name{ std::move(name) }
    , annotation{ std::move(annotation) }
    , expr_chain{ std::move(expr_chain) }
{
}

const declaration* port::resolve_annotation(const compilation_context& context) const
{
    if (!is_valid(context))
        return nullptr;

    if (!annotation || !declarer)
        return nullptr;

    return declarer->get_scope()->find(annotation->to_string(), true);
}

object_const_shared_ptr port::generate_placeholder(const compilation_context& context, int& placeholder_index) const
{
    const auto* type = resolve_annotation(context);
    if (!type)
        return nullptr;

    return type->generate_placeholder(context, placeholder_index);
}

bool port::is_valid(const compilation_context& context) const
{
    if (!validated)
        validate(context);

    return valid;
}

void port::validate(const compilation_context& context) const
{
    validated = true;
    valid = true;

    if (!annotation || !declarer)
        return;

    const auto* decl = resolve_annotation(context);
    if (!decl)
    {
        error(fmt::format("constraint '{}' could not be found", annotation->to_string()),
              ELEMENT_ERROR_NOT_A_CONSTRAINT,
              declarer->source_info)
            .log_once(context.get_logger());
        valid = false;
        return;
    }

    const auto* struct_decl = dynamic_cast<const struct_declaration*>(decl);
    const auto* constraint_decl = dynamic_cast<const constraint_declaration*>(decl);
    if (!struct_decl && !constraint_decl)
    {
        error(fmt::format("'{}' is not a struct nor a constraint", annotation->to_string()),
            ELEMENT_ERROR_NOT_A_CONSTRAINT,
            declarer->source_info).log_once(context.get_logger());
        valid = false;
    }
}
