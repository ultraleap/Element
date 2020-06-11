#pragma once

#include <utility>
#include <vector>

#include "element_object.hpp"
#include "ast/types.hpp"

namespace element
{
	struct type_annotation;

	struct port : element_object
	{
        std::string identifier;
		
        //TODO: JM - constraint matching
        std::unique_ptr<element_constraint> constraint;

        explicit port(std::string identifier = "")
    		: identifier{std::move(identifier)}
        { 
        }

        [[nodiscard]] std::string to_string() const override
        {
            return identifier;
        }
    };
}
