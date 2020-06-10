#pragma once

#include <string>

#include "typeutil.hpp"

namespace element
{
    struct element_object
	{
		virtual ~element_object() = default;
    	
        [[nodiscard]] virtual std::string to_string() const = 0;
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
