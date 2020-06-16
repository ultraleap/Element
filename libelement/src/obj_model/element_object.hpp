#pragma once

//STD
#include <string>
#include <memory>
#include <utility>
#include <vector>

namespace element
{
    struct compiled_expression;
    struct identifier_expression;
    struct indexing_expression;
    struct call_expression;

    struct identifier
    {
        explicit identifier(std::string value)
            : value{std::move(value)}
        {
        }

        static identifier unidentifier;

        identifier(identifier const& other) = default;
        identifier& operator=(identifier const& other) = default;

        identifier(identifier&& other) = default;
        identifier& operator=(identifier&& other) = default;

        ~identifier() = default;

        std::string value;
    };

    struct element_object
	{
		virtual ~element_object() = default;

        [[nodiscard]] virtual std::string to_string() const { return ""; }
        [[nodiscard]] virtual std::string to_code(int depth) const { return ""; }

        //TODO: Add constraints
        //bool matches_constraint(constraint& constraint);

        //todo: some kind of component architecture?
        [[nodiscard]] virtual std::shared_ptr<element_object> index(const identifier&) const { return nullptr; };
        [[nodiscard]] virtual std::shared_ptr<element_object> call(std::vector<std::shared_ptr<compiled_expression>> args) const { return nullptr; };
        [[nodiscard]] virtual std::shared_ptr<compiled_expression> compile() const { return nullptr; };
    };

 //   struct error : element_object
	//{
 //       static const object_model_id type_id;
 //   	
 //       std::string message;

 //       explicit error(std::string message)
 //   		: element_object(nullptr, type_id)
 //   		, message{std::move(message)}
 //       {
 //       }
 //   };
}
