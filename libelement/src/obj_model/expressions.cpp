#include "expressions.hpp"

//LIBS
#include <fmt/format.h>

//SELF
#include "etree/expressions.hpp"
#include "errors.hpp"
#include "interpreter_internal.hpp"
#include "intrinsics.hpp"

namespace element
{
    expression_chain::expression_chain(const declaration* declarer)
        : declarer{declarer}
    {
        assert(declarer);
    }

    std::shared_ptr<const object> expression_chain::call(
        const compilation_context& context,
        std::vector<std::shared_ptr<const object>> compiled_args,
        const source_information& source_info) const
    {
        //expression_chains are no longer called, instead, the callstack has the arguments and chains are compiled
        //todo: this should never occur, so keeping the assert is useful. return internal compiler error instead of failed call?
        assert(false);
        return object::call(context, compiled_args, source_info);
    }

    std::shared_ptr<const object> expression_chain::compile(const compilation_context& context,
                                                            const source_information& source_info) const
    {
        std::shared_ptr<const object> current = nullptr;
        for (const auto& expression : expressions)
        {
            auto previous = std::move(current); //for debugging
            current = expression->resolve(context, previous.get())->compile(context, source_info);
        }

        auto* err = dynamic_cast<const error*>(current.get());
        if (err)
            err->log_once(context.interpreter->logger.get());

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

    [[nodiscard]] std::shared_ptr<const object> identifier_expression::resolve(const compilation_context& context, const object* obj)
    {
        auto element = context.captures.find(parent->declarer->our_scope.get(), name, context, parent->source_info);
        if (element) return element;

        return build_error_and_log(context, source_info, error_message_code::failed_to_find_when_resolving_identifier_expr, name.value);
    }

    literal_expression::literal_expression(element_value value, const expression_chain* parent)
        : expression(parent)
        , value(value)
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<const object> literal_expression::resolve(const compilation_context& context, const object* obj)
    {
        return std::make_shared<element_expression_constant>(value);
    }

    indexing_expression::indexing_expression(identifier name, const expression_chain* parent)
        : expression(parent)
        , name(std::move(name))
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<const object> indexing_expression::resolve(const compilation_context& context, const object* obj)
    {
        auto element = obj->index(context, name, source_info);
        if (element) return element;

        return build_error_and_log(context, source_info, error_message_code::failed_to_find_when_resolving_indexing_expr, name.value, obj->typeof_info());
    }

    call_expression::call_expression(const expression_chain* parent)
        : expression(parent)
    {
        assert(parent);
    }

    [[nodiscard]] std::shared_ptr<const object> call_expression::resolve(const compilation_context& context, const object* obj)
    {
        std::vector<std::shared_ptr<const object>> compiled_arguments;
        for (const auto& arg : arguments)
            compiled_arguments.push_back(arg->compile(context, source_info));

        auto element = obj->call(context, std::move(compiled_arguments), source_info);
        if (element) return element;

        assert(!"internal compiler error");
        return build_error_and_log(context, source_info, error_message_code::invalid_errorless_call, parent->declarer->name.value);
    }

    lambda_expression::lambda_expression(const expression_chain* parent)
        : expression(parent)
        , name(unidentifier)
    {
        assert(parent);
    }

    std::shared_ptr<const object> lambda_expression::resolve(const compilation_context& context, const object* obj)
    {
        assert(!"lambdas are not implemented");
        throw;
    }
}