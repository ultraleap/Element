#include "expressions.hpp"

#include "etree/expressions.hpp"
#include "scope.hpp"
#include "intermediaries.hpp"

namespace element
{
    expression_chain::expression_chain(const declaration* declarer)
        : declarer{declarer}
    {
        assert(declarer);
    }

    std::string expression_chain::to_code(int depth) const
    {
        static auto accumulate = [](std::string accumulator, const std::unique_ptr<expression>& expression)
        {
            return std::move(accumulator) + expression->to_code();
        };

        return std::accumulate(std::next(std::begin(expressions)), std::end(expressions), expressions[0]->to_code(), accumulate);
    }

    std::shared_ptr<object> expression_chain::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args) const
    {
        //todo: can't think of a reason this would be empty, even without having a global, the declaration should have added itself
        assert(!context.stack.frames.empty());

        //todo: for now, just some error checking to make sure we're not doing something too crazy, but all of this is a problem
        assert(compiled_args.size() == context.stack.frames.back().compiled_arguments.size());

        return compile(context);
    }

    std::shared_ptr<object> expression_chain::compile(const compilation_context& context) const
    {
        if (expressions.empty())
            return nullptr; //todo: error_object

        std::shared_ptr<object> current = nullptr;
        for (const auto& expression : expressions)
        {
            auto previous = std::move(current);
            current = expression->resolve(context, std::move(previous));
            assert(current);
        }

        return current->compile(context);
    }

    expression::expression(const expression_chain* parent)
        : parent(parent)
    {
        assert(parent);
    }

    identifier_expression::identifier_expression(identifier name, const expression_chain* parent)
        : expression(parent)
        , name(std::move(name))
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<object> identifier_expression::resolve(const compilation_context& context, std::shared_ptr<object> obj)
    {
        if (obj)
        {
            assert(!"somehow an identifier was not the first thing in the chain");
            return nullptr;
        }

        if (!parent->declarer->our_scope)
        {
            assert(!"somehow an expression chains declarer has no scope");
            return nullptr;
        }

        //todo: all below this is broke innit

        const auto& enclosing_scope = parent->declarer->our_scope;
        auto found = enclosing_scope->find(name.value, false);
        if (found)
            return found->compile(context);

        //todo: make it nice and on the callstack itself
        for (auto it = context.stack.frames.rbegin(); it < context.stack.frames.rend(); ++it)
        {
            const auto& frame = *it;
            for (std::size_t i = 0; i < frame.function->inputs.size(); i++)
            {
                const auto& input = frame.function->inputs[i];
                if (input.name.value == name.value)
                {
                    if (i < frame.compiled_arguments.size())
                        return frame.compiled_arguments[i];
                    else
                        break;
                }
            }
        }

        //todo: this has to be merged with the callstack I think, i.e. each level of scopage has its own locals + arguments to do lookups with
        found = enclosing_scope->get_parent_scope()->find(name.value, true);
        if (found)
            return found->compile(context);

        assert(false);
        throw;
        return nullptr;
    }

    literal_expression::literal_expression(element_value value, const expression_chain* parent)
        : expression(parent)
        , value(value)
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<object> literal_expression::resolve(const compilation_context& context, std::shared_ptr<object> obj)
    {
        if (obj)
        {
            assert(!"somehow a literal was not the first thing in the chain");
            return nullptr;
        }

        if (!parent->declarer->our_scope)
        {
            assert(!"somehow an expression chains declarer has no scope");
            return nullptr;
        }

        return std::make_shared<element_expression_constant>(value);
    }

    indexing_expression::indexing_expression(identifier name, const expression_chain* parent)
        : expression(parent)
        , name(std::move(name))
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<object> indexing_expression::resolve(const compilation_context& context, std::shared_ptr<object> obj)
    {
        if (!obj)
        {
            assert(!"somehow an indexing expression was the first thing in the chain");
            return nullptr;
        }

        return obj->index(context, name);
    }

    call_expression::call_expression(const expression_chain* parent)
        : expression(parent)
    {
        assert(parent);
    }

    std::string call_expression::to_code(int depth) const
    {
        static auto accumulate = [](std::string accumulator, const std::unique_ptr<expression_chain>& chain)
        {
            return std::move(accumulator) + ", " + chain->to_code();
        };

        const auto expressions = std::accumulate(std::next(std::begin(arguments)), std::end(arguments), arguments[0]->to_code(), accumulate);
        return "(" + expressions + ")";
    }

    [[nodiscard]] std::shared_ptr<object> call_expression::resolve(const compilation_context& context, std::shared_ptr<object> obj)
    {
        if (!obj)
        {
            assert(!"somehow a call expression was the first thing in the chain");
            return nullptr;
        }

        std::vector<std::shared_ptr<object>> compiled_arguments;
        for (const auto& arg : arguments)
            compiled_arguments.push_back(arg->compile(context));

        return obj->call(context, std::move(compiled_arguments));
    }

    /*
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
    }*/
}