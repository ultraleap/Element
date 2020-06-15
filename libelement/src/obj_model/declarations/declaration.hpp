#pragma once

#include <memory>
#include <memory>
#include <string>
#include <vector>


#include "obj_model/port.hpp"
#include "obj_model/element_object.hpp"
#include "obj_model/expressions/expression.hpp"

namespace element
{
    struct scope;

    static const std::string intrinsic_qualifier = "intrinsic";
	static const std::string namespace_qualifier = "namespace";
    static const std::string constraint_qualifier = "constraint";
    static const std::string struct_qualifier = "struct";
    static const std::string function_qualifier; //empty string
    static const std::string return_keyword = "return";
	
	struct declaration : element_object
	{
        std::vector<port> inputs;
        std::unique_ptr<port> output;
        //std::unique_ptr<element_constraint> constraint;
		
		std::string qualifier;
        identifier identifier;
        bool intrinsic = false;

        std::unique_ptr<scope> scope; //needed to merge object model

        explicit declaration();
		
        [[nodiscard]] bool has_inputs() const;
        [[nodiscard]] bool has_output() const;
        [[nodiscard]] bool has_constraint() const; //TODO: JM - nonsense, this needs to be a constraint::something OR constraint::any
        [[nodiscard]] bool is_intrinsic() const;
        [[nodiscard]] virtual std::string location() const;
    };

    struct scoped_declaration : declaration
	{
        [[nodiscard]] bool has_scope() const;
    	
        explicit scoped_declaration(const element::scope* parent_scope);

    	void add_declaration(std::shared_ptr<declaration> declaration) const;

        [[nodiscard]] std::string location() const override;
    };

    struct struct_declaration final : scoped_declaration
	{
        struct_declaration(const element::scope* parent_scope, bool is_intrinsic);
    	
        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] std::shared_ptr<element_object> index(const element::identifier&) const override;
    };
	
    struct constraint_declaration final : declaration
	{
		constraint_declaration(bool is_intrinsic);
    };

    struct function_declaration final : scoped_declaration
	{
        function_declaration(const element::scope* parent_scope, bool is_intrinsic);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] std::shared_ptr<element_object> call(std::vector<std::shared_ptr<compiled_expression>> args) const override;
        [[nodiscard]] std::shared_ptr<compiled_expression> compile() const override;
    };

    //expression bodied functions are used as the leaf-functions for a chain of scope bodied ones to prevent recursion
    //the last thing in a function call chain must be an expression bodied "return"
    struct expression_bodied_function_declaration final : scoped_declaration {

        std::shared_ptr<expression> expression;
    	
        expression_bodied_function_declaration(const element::scope* parent_scope);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] std::shared_ptr<element_object> call(std::vector<std::shared_ptr<compiled_expression>> args) const override;
        [[nodiscard]] std::shared_ptr<compiled_expression> compile() const override;
    };

    struct namespace_declaration final : scoped_declaration
	{
        namespace_declaration(const element::scope* parent_scope);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] std::shared_ptr<element_object> index(const element::identifier&) const override;
    };
}
