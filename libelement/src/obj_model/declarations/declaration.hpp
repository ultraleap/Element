#pragma once

#include <memory>
#include <memory>
#include <string>
#include <utility>

#include "obj_model/port.hpp"
#include "obj_model/element_object.hpp"

namespace element
{
    struct scope;
	
	static const std::string namespace_qualifier = "namespace";
    static const std::string struct_qualifier = "struct";
    static const std::string constraint_qualifier = "constraint";
    static const std::string function_qualifier = "function";
	
	struct declaration : element_object
	{
        std::vector<port> inputs;
        port output;
        std::string qualifier;
        std::string identifier;
        bool intrinsic;

        explicit declaration();
    };

    struct scoped_declaration : declaration {
    	
        std::shared_ptr<scope> scope;

        [[nodiscard]] bool has_scope() const;
        [[nodiscard]] bool has_inputs() const;
    	
        explicit scoped_declaration(const std::shared_ptr<element::scope>& parent_scope);

    	void add_declaration(std::unique_ptr<declaration> declaration) const;

        [[nodiscard]] std::string to_string() const override;
    };

    struct struct_declaration : scoped_declaration
	{
        struct_declaration(const std::shared_ptr<element::scope>& parent_scope, bool is_intrinsic);
    };
	
    struct constraint_declaration : declaration
	{
		constraint_declaration(bool is_intrinsic);
    };

    struct function_declaration : scoped_declaration
	{
        function_declaration(const std::shared_ptr<element::scope>& parent_scope, bool is_intrinsic);
    };

    struct namespace_declaration : scoped_declaration
	{
        namespace_declaration(std::shared_ptr<element::scope> parent_scope);
    };
}
