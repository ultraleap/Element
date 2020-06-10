#pragma once

#include <vector>
#include <memory>

#include "obj_model/element_object.hpp"
#include "obj_model/declarations/declaration.hpp"

namespace element
{
    struct scope : element_object
	{
        std::shared_ptr<scope> parent_scope;
        const declaration* const declarer;
    	
        std::vector<std::unique_ptr<declaration>> declarations;

        explicit scope(std::shared_ptr<scope> parent_scope, const declaration* const declarer)
            : parent_scope{ std::move(parent_scope) }, declarer{ declarer }
        {
        }
    	
        void add_declaration(std::unique_ptr<declaration> declaration);

        [[nodiscard]] bool is_root() const
    	{
            return parent_scope == nullptr;
    	}
    	
        [[nodiscard]] std::string location() const;
        [[nodiscard]] std::string to_string() const override;
    };

    struct root_scope final : scope
	{
        explicit root_scope()
            : scope(nullptr, nullptr)
        {
        }
    };
}
