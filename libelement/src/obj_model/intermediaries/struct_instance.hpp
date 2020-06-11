#pragma once

#include "obj_model/scopes/scope.hpp"

namespace element
{
    struct struct_instance : element_object
    {
        const struct_declaration* const declarer;
        std::map<std::string, std::shared_ptr<expression>> fields;

        explicit struct_instance(const element::struct_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions);

        [[nodiscard]] std::string to_string() const override;
    };
}
