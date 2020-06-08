#pragma once

namespace element::object_model
{
    struct element_object {
    	
        element_object* parent;
    	
        explicit element_object(element_object* parent): parent {parent}
        { 
        }
    };
}
