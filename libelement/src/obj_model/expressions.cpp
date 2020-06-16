#include "expressions.hpp"

#include "etree/expressions.hpp"
#include "obj_model/scope.hpp"
#include "obj_model/intermediaries.hpp"

namespace element
{
    expression::expression(const scope* enclosing_scope)
        : enclosing_scope{ enclosing_scope }
    {
    }

    std::string expression::to_code(int depth) const
    {
        static auto accumulate = [](std::string accumulator, const std::shared_ptr<expression>& expression)
        {
            return std::move(accumulator) + expression->to_code();
        };

        return std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_code(), accumulate);
    }

    std::shared_ptr<compiled_expression> expression::compile(const compilation_context& context) const
    {
        if (children.empty())
            return nullptr; //todo: error_object

        //find first thing in the chain
        std::shared_ptr<object> current = nullptr;
        for (const auto& expression : children)
        {
            current = expression->compile(context, current);
        }

        if (dynamic_cast<compiled_expression*>(current.get()))
        {
            return std::static_pointer_cast<compiled_expression>(current);
        }

        throw; //todo: could be e.g. a namespace declaration, so not something that was compilable
        return nullptr;
    }

    std::shared_ptr<object> literal_expression::index(const compilation_context& context, const identifier& identifier) const
    {
        const auto& num = enclosing_scope->get_global()->find("Num", false);
        const auto obj = num->index(context, identifier);
        if (dynamic_cast<function_declaration*>(obj.get()))
        {
            auto func = static_cast<function_declaration*>(obj.get());
            //todo: typechecking

            //hopefully this is a sufficiently large warning sign that what we're doing here is not good
            auto compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation = std::make_shared<compiled_expression>();
            compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation->expression_tree = std::make_shared<element_expression_constant>(value);
            compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation->object_model = std::make_shared<literal_expression>(value, enclosing_scope); //this is really bad, we should not recreate the literal expression. all of this is an iffy hack though
            compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation->creator = enclosing_scope->get_global()->find("Num", false).get();
            std::vector<std::shared_ptr<compiled_expression>> compiled_args = { { std::move(compile_ourselves_again_because_we_dont_have_access_to_our_original_compilation) } };
            auto partially_applied_function = std::make_shared<function_instance>(func, std::move(compiled_args));
            return std::move(partially_applied_function);
        }

        throw;
        return nullptr;
    }

    [[nodiscard]] std::shared_ptr<object> identifier_expression::compile(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (previous) //cannot resolve identifier if previous exists
            return nullptr;

        if (!enclosing_scope)
            return nullptr;

        return enclosing_scope->find(identifier.value, true);
    }

    [[nodiscard]] std::shared_ptr<object> literal_expression::compile(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (previous) //cannot resolve literal if previous exists
            return nullptr;

        if (!enclosing_scope)
            return nullptr;

        auto compiled = std::make_shared<compiled_expression>();
        compiled->expression_tree = std::make_shared<element_expression_constant>(value);
        //a compiled expression's declarer is always a raw pointer, because it shouldn't be an intermediary but something that's part of the object model. I think
        //it's basically just some root thing we can use to track down where it came from if we need to
        compiled->object_model = shared_from_this();
        //todo: don't have constraints set up right now, so just hacking it in to declarer. not even sure if that's a bad thing, declarer makes sense?
        compiled->creator = enclosing_scope->get_global()->find("Num", false).get(); //hopefully the Num declaration doesn't move anywhere (e.g merging..)
        return std::move(compiled);
    }

    [[nodiscard]] std::shared_ptr<object> indexing_expression::compile(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (!previous) //can only resolve indexing if previous exists
            return nullptr;

        return previous->index(context, name);
    }

    std::string call_expression::to_code(int depth) const
    {
        static auto accumulate = [](std::string accumulator, const std::shared_ptr<expression>& expression)
        {
            return std::move(accumulator) + ", " + expression->to_code();
        };

        const auto expressions = std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_code(), accumulate);
        return "(" + expressions + ")";
    }

    [[nodiscard]] std::shared_ptr<object> call_expression::compile(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (!previous) //can only resolve call if previous exists
            return nullptr;

        std::vector<std::shared_ptr<compiled_expression>> compiled_arguments;
        for (const auto& arg : children)
        {
            compiled_arguments.push_back(arg->compile(context));
        }

        return previous->call(context, std::move(compiled_arguments));
    }

    lambda_expression::lambda_expression(const scope* parent_scope)
        : expression(parent_scope)
    {

    }

    [[nodiscard]] std::string lambda_expression::to_string() const
    {
        return "_";
    }

    [[nodiscard]] std::string lambda_expression::to_code(int depth) const
    {
        return "_";
    }

    expression_bodied_lambda_expression::expression_bodied_lambda_expression(const scope* parent_scope)
        : lambda_expression(parent_scope)
    {
    }

    [[nodiscard]] std::string expression_bodied_lambda_expression::to_string() const
    {
        return "_";
    }

    [[nodiscard]] std::string expression_bodied_lambda_expression::to_code(int depth) const
    {
        return "_";
    }
}