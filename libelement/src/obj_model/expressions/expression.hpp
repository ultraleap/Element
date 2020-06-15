#pragma once

#include <numeric>
#include <utility>
#include <vector>

#include "element/common.h"
#include "ast/types.hpp"
#include "obj_model/element_object.hpp"
#include "obj_model/port.hpp"

namespace element
{
    struct scope;

    struct expression : element_object, std::enable_shared_from_this<expression>
	{
        const scope* enclosing_scope;

    	//need to use a shared pointer here as call expressions can have a list of independent expressions that all share the same parent i.e. the call expression itself
        std::vector<std::shared_ptr<expression>> children;

        expression(const scope* enclosing_scope);

        [[nodiscard]] virtual bool has_children() const { return !children.empty();  }
        [[nodiscard]] std::string to_code(int depth = 0) const override
        {
            static auto accumulate = [](std::string accumulator, const std::shared_ptr<element::expression>& expression)
            {
                return std::move(accumulator) + expression->to_code();
            };

            return std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_code(), accumulate);
        }

        [[nodiscard]] std::shared_ptr<compiled_expression> compile() const override;

        [[nodiscard]] virtual std::shared_ptr<element_object> compile(std::shared_ptr<element_object>) { return nullptr; }
    };

	struct literal_expression final : expression
	{
        element_value value;

        explicit literal_expression(const element_value value, const scope* enclosing_scope)
			: expression{ enclosing_scope }, value{value}
        {
        }

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_code(int depth = 0) const override { return std::to_string(value); }

        //when a literal is compiled and we need to later index it, it goes through here
        [[nodiscard]] std::shared_ptr<element_object> index(const identifier&) const override;
        [[nodiscard]] std::shared_ptr<element_object> compile(std::shared_ptr<element_object> previous) override;
	};

    struct identifier_expression final : expression
	{
        identifier identifier;

        explicit identifier_expression(std::string identifier, const scope* enclosing_scope)
            : expression{ enclosing_scope }, identifier{std::move(identifier)}
        {
        }

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_code(int depth = 0) const override { return identifier.value; }
        //[[nodiscard]] const element_object* index(const indexing_expression*) const override;
        [[nodiscard]] std::shared_ptr<element_object> compile(std::shared_ptr<element_object> previous) override;
    };

    struct call_expression final : expression
	{
        explicit call_expression(const scope* enclosing_scope)
            : expression{ enclosing_scope }
        {
        }

        [[nodiscard]] std::string to_code(int depth = 0) const override
        {
            static auto accumulate = [](std::string accumulator, const std::shared_ptr<element::expression>& expression)
            {
                return std::move(accumulator) + ", " + expression->to_code();
            };

            const auto expressions = std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_code(), accumulate);
        	return "(" + expressions + ")";
        }

        //[[nodiscard]] const element_object* index(const indexing_expression*) const override;
        [[nodiscard]] std::shared_ptr<element_object> compile(std::shared_ptr<element_object> previous) override;
    };
	
    struct indexing_expression final : expression
	{
        identifier identifier;

        explicit indexing_expression(std::string identifier, const scope* enclosing_scope)
            : expression{ enclosing_scope }, identifier{ std::move(identifier) }
        {
        }

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_code(int depth = 0) const override
        {
	        return "." + identifier.value;
        }

        [[nodiscard]] std::shared_ptr<element_object> compile(std::shared_ptr<element_object> previous) override;
    };
	
    struct lambda_expression final : expression
	{
		//TODO
        std::vector<port> inputs;
        std::unique_ptr<port> output;
        //std::unique_ptr<element_constraint> constraint;
    };
}
