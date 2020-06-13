#include "expression.hpp"

#include "etree/expressions.hpp"
#include "obj_model/scopes/scope.hpp"
#include "obj_model/intermediaries/struct_instance.hpp"

element::expression::expression(const scope* enclosing_scope)
    : enclosing_scope{enclosing_scope}
{
}

std::shared_ptr<element::compiled_expression> element::expression::compile() const
{
    if (children.empty())
        return nullptr; //todo: error_object

    //find first thing in the chain
    std::shared_ptr<element_object> current = children[0];

    //todo: ew dynamic casts. fix and think of a nicer solution to doing a dynamic and then a static
    if (dynamic_cast<const identifier_expression*>(current.get()))
    {
        const auto* const expr = static_cast<const identifier_expression*>(current.get());
        current = expr->enclosing_scope->find(expr->identifier, true);
    }
    else if (dynamic_cast<const literal_expression*>(current.get()))
    {
        const auto* const expr = static_cast<const literal_expression*>(current.get());
        auto compiled = std::make_shared<compiled_expression>();
        compiled->expression = std::make_shared<element_expression_constant>(expr->value);
        //a compiled expression's declarer is always a raw pointer, because it shouldn't be an intermediary but something that's part of the object model. I think
        //it's basically just some root thing we can use to track down where it came from if we need to
        compiled->declarer = current.get();
        current = std::move(compiled);
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
            std::vector<std::shared_ptr<compiled_expression>> compiled_arguments;
            for (auto& arg : expr->children)
            {
                compiled_arguments.push_back(arg->compile());
            }
            current = current->call(std::move(compiled_arguments));
        }
        else
        {
            return nullptr; //todo:
        }
    }

    if (dynamic_cast<compiled_expression*>(current.get()))
    {
        return std::static_pointer_cast<compiled_expression>(current);
    }

    throw; //todo: could be e.g. a namespace declaration, so not something that was compileable
    return nullptr;
}
