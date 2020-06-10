#pragma once

#include <vector>
#include <memory>

#include "obj_model/element_object.hpp"
#include "obj_model/declarations/declaration.hpp"

namespace element
{
    struct scope : element_object
	{
        const scope* const parent_scope = nullptr;
        const declaration* const declarer;
    	
        std::vector<std::unique_ptr<declaration>> declarations;

        explicit scope(const scope* parent_scope, const declaration* const declarer)
            : parent_scope{ parent_scope }, declarer{ declarer }
        {
        }
    	
        void add_declaration(std::unique_ptr<declaration> declaration);

        [[nodiscard]] bool is_root() const
    	{
            assert(!declarer);
            return parent_scope == nullptr;
    	}
    	
        [[nodiscard]] std::string location() const;
        [[nodiscard]] std::string to_string() const override;
    };
}
