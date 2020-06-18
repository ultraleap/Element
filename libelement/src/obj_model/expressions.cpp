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

    std::shared_ptr<element_expression> expression::compile(const compilation_context& context) const
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

        //todo: all below this is broke innit
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

        std::vector<std::shared_ptr<element_expression>> compiled_arguments;
        for (const auto& arg : children)
            compiled_arguments.push_back(arg->compile(context));

        call_stack::frame frame;
        frame.arguments = std::move(compiled_arguments);
        context.stack.frames.emplace_back(std::move(frame));
        auto ret =  previous->call(context, context.stack.frames.back().arguments);
        context.stack.frames.pop_back();
        return ret;
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

element_expression_constant::element_expression_constant(element_value val) : element_expression(type_id), m_value(val)
{
    type = element::type::num;
}

std::shared_ptr<element_expression> element_expression::compile(const element::compilation_context& context) const
{
    //TODO: THIS IS AFWUL! FIX!
    return std::dynamic_pointer_cast<element_expression>(const_cast<element_expression*>(this)->shared_from_this());
}

std::shared_ptr<element::object> element_expression::index(const element::compilation_context& context, const element::identifier& identifier) const
{
    assert(type);

    const auto declarer = type->index(context, identifier);
    auto args = std::vector<std::shared_ptr<element_expression>>();
    args.push_back(const_cast<element_expression*>(this)->shared_from_this());

    auto* function_declaration = dynamic_cast<element::function_declaration*>(declarer.get());
    if(function_declaration)
        return std::make_shared<element::function_instance>(function_declaration, args);

    throw;
}