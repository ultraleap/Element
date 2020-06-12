#include "../scopes/scope.hpp"
#include "expression.hpp"

#include "obj_model/intermediaries/struct_instance.hpp"

element::expression::expression(const scope* enclosing_scope)
    : enclosing_scope{enclosing_scope}
{
}

std::unique_ptr<element::compiled_expression> element::expression::compile()
{
    if (children.empty())
        return nullptr; //todo: error_object

    //find first thing in the chain
    const element_object* current = children[0].get();
    //todo: ew dynamic casts

    if (dynamic_cast<const identifier_expression*>(current))
    {
        const auto* const expr = static_cast<const identifier_expression*>(current);
        current = expr->enclosing_scope->find(expr->identifier, true);
    }
    else if (dynamic_cast<const literal_expression*>(current))
    {
        const auto* const expr = static_cast<const literal_expression*>(current);
        current = nullptr; //todo
    }
    else
    {
        return nullptr; //todo
    }

    //do indexing and calling
    for (std::size_t i = 1; i < children.size(); i++)
    {
        auto* const child = children[i].get();

        if (dynamic_cast<const indexing_expression*>(child))
        {
            const auto* const expr = static_cast<const indexing_expression*>(child);
            current = current->index(expr);
        }
        else if (dynamic_cast<const call_expression*>(child))
        {
            const auto* const expr = static_cast<const call_expression*>(child);
            current = current->call(expr);
        }
        else
        {
            return nullptr; //todo:
        }
    }


    //TODO: return a thing
    return std::make_unique<compiled_expression>();
}
