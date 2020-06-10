#pragma once

#include <utility>
#include <vector>


#include "ast/types.hpp"
#include "obj_model/element_object.hpp"
#include "obj_model/port.hpp"

namespace element
{
    struct expression : element_object
	{
    	//need to use a shared pointer here as call expressions can have a list of independent expressions that all share the same parent i.e. the call expression itself
        std::vector<std::shared_ptr<expression>> children;

        [[nodiscard]] std::string to_string() const override
        {
            static auto accumulate = [](std::string accumulator, const std::shared_ptr<element::expression>& expression)
            {
                return std::move(accumulator) + expression->to_string();
            };

            return std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_string(), accumulate);
        }
    };

	struct literal_expression final : expression
	{
        float value;

        explicit literal_expression(float value)
			: value{value}
        {
        }

        [[nodiscard]] std::string to_string() const override { return std::to_string(value); }
	};

    struct identifier_expression final : expression
	{
        std::string identifier;

        explicit identifier_expression(std::string identifier)
            : identifier{std::move(identifier)}
        {
        }

        [[nodiscard]] std::string to_string() const override { return identifier; }
    };

    struct call_expression final : expression
	{
        [[nodiscard]] std::string to_string() const override
        {
            static auto accumulate = [](std::string accumulator, const std::shared_ptr<element::expression>& expression)
            {
                return std::move(accumulator) + "," + expression->to_string();
            };

            const auto expressions = std::accumulate(std::next(std::begin(children)), std::end(children), children[0]->to_string(), accumulate);
        	return "(" + expressions + ")";
        }
    };
	
    struct indexing_expression final : expression
	{
        std::string identifier;

        explicit indexing_expression(std::string identifier)
            : identifier{std::move(identifier)}
        {
        }

        [[nodiscard]] std::string to_string() const override
        {
	        return "." + identifier;
        }
    };
	
    struct lambda_expression final : expression
	{
        std::vector<port> inputs;
        std::unique_ptr<port> output;
        std::unique_ptr<element_constraint> constraint;
    };
}
