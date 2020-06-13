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

    struct expression : element_object
	{
        const scope* enclosing_scope;

    	//need to use a shared pointer here as call expressions can have a list of independent expressions that all share the same parent i.e. the call expression itself
        std::vector<std::shared_ptr<expression>> children;

        expression(const scope* enclosing_scope);

        [[nodiscard]] virtual bool has_children() const { return !children.empty();  }
        [[nodiscard]] std::string to_string() const override
        {
            static auto accumulate = [](std::string accumulator, const std::shared_ptr<element::expression>& expression)
            {
                return std::move(accumulator) + expression->to_string();
            };

            return std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_string(), accumulate);
        }

        [[nodiscard]] std::shared_ptr<compiled_expression> compile() const override;
    };

	struct literal_expression final : expression
	{
        element_value value;

        explicit literal_expression(const element_value value, const scope* enclosing_scope)
			: expression{ enclosing_scope }, value{value}
        {
        }

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_string() const override { return std::to_string(value); }

        //when a literal is compiled and we need to later index it, it goes through here
        [[nodiscard]] std::shared_ptr<element_object> index(const indexing_expression*) const override;
	};

    struct identifier_expression final : expression
	{
        std::string identifier;

        explicit identifier_expression(std::string identifier, const scope* enclosing_scope)
            : expression{ enclosing_scope }, identifier{std::move(identifier)}
        {
        }

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_string() const override { return identifier; }
        //[[nodiscard]] const element_object* index(const indexing_expression*) const override;
    };

    struct call_expression final : expression
	{
        explicit call_expression(const scope* enclosing_scope)
            : expression{ enclosing_scope }
        {
        }

        [[nodiscard]] std::string to_string() const override
        {
            static auto accumulate = [](std::string accumulator, const std::shared_ptr<element::expression>& expression)
            {
                return std::move(accumulator) + "," + expression->to_string();
            };

            const auto expressions = std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_string(), accumulate);
        	return "(" + expressions + ")";
        }

        //[[nodiscard]] const element_object* index(const indexing_expression*) const override;
    };
	
    struct indexing_expression final : expression
	{
        std::string identifier;

        explicit indexing_expression(std::string identifier, const scope* enclosing_scope)
            : expression{ enclosing_scope }, identifier{ std::move(identifier) }
        {
        }

        [[nodiscard]] bool has_children() const override { return false; }
        [[nodiscard]] std::string to_string() const override
        {
	        return "." + identifier;
        }
    };
	
    struct lambda_expression final : expression
	{
		//TODO
        std::vector<port> inputs;
        std::unique_ptr<port> output;
        //std::unique_ptr<element_constraint> constraint;
    };
}
