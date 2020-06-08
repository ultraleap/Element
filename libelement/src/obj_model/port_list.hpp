#pragma once

#include <vector>

#include "element_object.hpp"
#include "port.hpp"

namespace element::object_model
{
    struct port_list : public element_object
	{
        std::vector<port> ports;
    	
        explicit port_list(element_object* parent) :
    		element_object(parent), ports{ std::move(ports) }
        {
        }
    };
}
