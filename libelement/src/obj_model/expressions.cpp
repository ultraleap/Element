#include "expressions.hpp"

#include "etree/expressions.hpp"
#include "scope.hpp"
#include "intermediaries.hpp"

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

    std::shared_ptr<object> expression::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args) const
    {
        //todo: can't think of a reason this would be empty, even without having a global, the declaration should have added itself
        assert(!context.stack.frames.empty());

        //todo: for now, just some error checking to make sure we're not doing something too crazy, but all of this is a problem
        assert(compiled_args.size() == context.stack.frames.back().compiled_arguments.size());

        return compile(context);
    }

    std::shared_ptr<object> expression::compile(const compilation_context& context) const
    {
        if (children.empty())
            return nullptr; //todo: error_object

        //find first thing in the chain
        std::shared_ptr<object> current = nullptr;
        for (const auto& expression : children)
        {
            auto previous = std::move(current);
            current = expression->resolve_expression(context, previous);
            assert(current);
        }

        //if (dynamic_cast<element_expression*>(current.get()))
        //{
        //    return dynamic_cast<element_expression*>(current.get());
        //}

        return current->compile(context);
    }

    [[nodiscard]] std::shared_ptr<object> identifier_expression::resolve_expression(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (previous) //cannot resolve identifier if previous exists
            return nullptr;

        if (!enclosing_scope)
            return nullptr;


        //todo: all below this is broke innit

        auto found = enclosing_scope->find(identifier.value, false);
        if (found)
            return found;

        //todo: make it nice and on the callstack itself
        for (auto it = context.stack.frames.rbegin(); it < context.stack.frames.rend(); ++it)
        {
            const auto& frame = *it;
            for (std::size_t i = 0; i < frame.function->inputs.size(); i++)
            {
                const auto& input = frame.function->inputs[i];
                if (input.name.value == identifier.value)
                {
                    if (i < frame.compiled_arguments.size())
                        return frame.compiled_arguments[i];
                    else
                        break;
                }
            }
        }

        //todo: this has to be merged with the callstack I think, i.e. each level of scopage has its own locals + arguments to do lookups with
        found = enclosing_scope->get_parent_scope()->find(identifier.value, true);
        if (found)
            return found;

        assert(false);
        throw;
        return nullptr;
    }

    [[nodiscard]] std::shared_ptr<object> literal_expression::resolve_expression(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (previous) //cannot resolve literal if previous exists
            return nullptr;

        if (!enclosing_scope)
            return nullptr;

        return std::make_shared<element_expression_constant>(value);
    }

    [[nodiscard]] std::shared_ptr<object> indexing_expression::resolve_expression(const compilation_context& context, std::shared_ptr<object> previous)
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

    [[nodiscard]] std::shared_ptr<object> call_expression::resolve_expression(const compilation_context& context, std::shared_ptr<object> previous)
    {
        if (!previous) //can only resolve call if previous exists
            return nullptr;

        std::vector<std::shared_ptr<object>> compiled_arguments;
        for (const auto& arg : children)
            compiled_arguments.push_back(arg->compile(context));

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

element_expression_if::element_expression_if(expression_shared_ptr predicate, expression_shared_ptr if_true, expression_shared_ptr if_false)
    : element_expression(type_id, element::type::boolean)
{
    m_dependents.emplace_back(std::move(predicate));
    m_dependents.emplace_back(std::move(if_true));
    m_dependents.emplace_back(std::move(if_false));
}