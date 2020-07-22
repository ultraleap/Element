#include "expressions.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "etree/expressions.hpp"
#include "scope.hpp"
#include "errors.hpp"

namespace element
{
    expression_chain::expression_chain(const declaration* declarer)
        : declarer{declarer}
    {
        assert(declarer);
    }

    std::shared_ptr<object> expression_chain::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<object>> compiled_args,
        const source_information& source_info) const
    {
        //expression_chains are no longer called, instead, the callstack has the arguments and chains are compiled
        assert(false);
        return object::call(context, compiled_args, source_info);
    }

    std::shared_ptr<object> expression_chain::compile(const compilation_context& context, const source_information& source_info) const
    {
        if (expressions.empty())
            return nullptr; //todo: error_object

        std::shared_ptr<object> current = nullptr;
        for (const auto& expression : expressions)
        {
            auto previous = std::move(current); //for debugging
            current = expression->resolve(context, previous)->compile(context, source_info);
        }

        return current;
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

    [[nodiscard]] std::shared_ptr<object> identifier_expression::resolve(const compilation_context& context, std::shared_ptr<object>& obj)
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
            return found->compile(context, source_info);

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
            return found->compile(context, source_info);

        return build_error(source_info, error_message_code::failed_to_find_when_resolving_identifier_expr, name.value);
    }

    literal_expression::literal_expression(element_value value, const expression_chain* parent)
        : expression(parent)
        , value(value)
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<object> literal_expression::resolve(const compilation_context& context, std::shared_ptr<object>& obj)
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

    [[nodiscard]] std::shared_ptr<object> indexing_expression::resolve(const compilation_context& context, std::shared_ptr<object>& obj)
    {
        if (!obj)
        {
            assert(!"somehow an indexing expression was the first thing in the chain");
            return nullptr;
        }

        auto element = obj->index(context, name, source_info);

        if (!element)
            return build_error(source_info, error_message_code::failed_to_find_when_resolving_indexing_expr, name.value, obj->typeof_info());

        return element;
    }

    call_expression::call_expression(const expression_chain* parent)
        : expression(parent)
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<object> call_expression::resolve(const compilation_context& context, std::shared_ptr<object>& obj)
    {
        if (!obj)
        {
            assert(!"somehow a call expression was the first thing in the chain");
            return nullptr;
        }

        std::vector<std::shared_ptr<object>> compiled_arguments;
        for (const auto& arg : arguments)
            compiled_arguments.push_back(arg->compile(context, source_info));

        auto ret = obj->call(context, std::move(compiled_arguments), source_info);
        if (ret)
            return ret;

        //todo: build error, but in theory this shouldn't happen
        assert(false);
        return nullptr;
    }

    lambda_expression::lambda_expression(const expression_chain* parent)
        : expression(parent)
        , name(unidentifier)
    {
        assert(parent);
    }

    std::shared_ptr<object> lambda_expression::resolve(const compilation_context& context, std::shared_ptr<object>& obj)
    {
        throw;
    }
}