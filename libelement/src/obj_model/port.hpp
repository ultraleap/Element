#pragma once

#include <utility>
#include <vector>

#include "element_object.hpp"

namespace element
{
    struct port : element_object
	{
        std::string identifier;
    	//TODO: type annotation & constraint matching

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
