#pragma once

#include "obj_model/scopes/scope.hpp"

namespace element
{

    //todo: cba to make new headers atm
    struct compiled_expression : element_object
    {
        const element_object* source = nullptr; //literal, struct instance, function instance?
        constraint_const_shared_ptr constraint = element_type::any;
        const expression_shared_ptr expression;

        [[nodiscard]] const element_object* index(const indexing_expression*) const override;
    };

    struct struct_instance : element_object
    {
        const struct_declaration* const declarer;
        std::map<std::string, std::unique_ptr<compiled_expression>> fields;

        explicit struct_instance(const element::struct_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions);

        [[nodiscard]] std::string to_string() const override;

        [[nodiscard]] const element_object* index(const indexing_expression*) const override;
    };

    struct function_instance : element_object
    {
        const function_declaration* const source;
        std::map<std::string, std::unique_ptr<compiled_expression>> provided_arguments;

        explicit function_instance(const function_declaration* source, const std::vector<std::shared_ptr<expression>>& expressions);

        [[nodiscard]] bool is_instance_function() const;
        [[nodiscard]] std::string to_string() const override;

        [[nodiscard]] const element_object* call(const call_expression*);
    };
}
