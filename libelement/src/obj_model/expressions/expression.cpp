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
        compiled->object = current;
        //todo: don't have constraints set up right now, so just hacking it in to declarer. not even sure if that's a bad thing, declarer makes sense?
        compiled->declarer = enclosing_scope->get_global()->find("Num", false).get(); //hopefully the Num declaration doesn't move anywhere (e.g merging..)
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

std::shared_ptr<element::element_object> element::literal_expression::index(const indexing_expression* expr) const
{
    const auto& num = enclosing_scope->get_global()->find("Num", false);
    const auto obj = num->index(expr);
    if (dynamic_cast<function_declaration*>(obj.get()))
    {
        auto func = static_cast<function_declaration*>(obj.get());
        //todo: typechecking

        //hopefully this is a sufficiently large warning sign that what we're doing here is not good
        auto compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation = std::make_shared<compiled_expression>();
        compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation->expression = std::make_shared<element_expression_constant>(value);
        compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation->object = std::make_shared<literal_expression>(value, enclosing_scope); //this is really bad, we should not recreate the literal expression. all of this is an iffy hack though
        compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation->declarer = enclosing_scope->get_global()->find("Num", false).get();
        std::vector<std::shared_ptr<compiled_expression>> compiled_args = { { std::move(compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation) } };
        auto partially_applied_function = std::make_shared<function_instance>(func, std::move(compiled_args));
        return std::move(partially_applied_function);
    }

    throw;
    return nullptr;
}