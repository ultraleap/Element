#pragma once

#include <utility>

#include "type_annotation.hpp"
#include "element_object.hpp"
#include "ast/types.hpp"

namespace element
{
	struct type_annotation;

	struct port : element_object
	{
        identifier identifier;
        std::unique_ptr<type_annotation> annotation;

        explicit port(element::identifier identifier, std::unique_ptr<type_annotation> annotation);

        [[nodiscard]] std::string to_string() const override { return identifier.value; }
        [[nodiscard]] std::string to_code(int depth) const override
        {
            if(annotation)
                return annotation->to_code(depth);

            return "";
        }
    };
}
