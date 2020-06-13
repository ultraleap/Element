#pragma once

#include "obj_model/scopes/scope.hpp"
#include "etree/fwd.hpp"

namespace element
{
    struct compiled_expression final : element_object
    {
        const element_object* declarer = nullptr; //the thing that generated this thing
        std::shared_ptr<element_object> object; //literal, struct instance, function instance?
        //constraint_const_shared_ptr constraint = element_type::any;
        expression_shared_ptr expression;

        [[nodiscard]] std::shared_ptr<element_object> index(const indexing_expression*) const override;
        [[nodiscard]] std::string to_string() const override { return ""; }
    };

    struct struct_instance final : element_object
    {
        const struct_declaration* const declarer;
        std::map<std::string, std::unique_ptr<compiled_expression>> fields;

        explicit struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::shared_ptr<element_object> index(const indexing_expression*) const override;
    };

    struct function_instance final : element_object
    {
        const function_declaration* const declarer;
        std::map<std::string, std::unique_ptr<compiled_expression>> provided_arguments;

        explicit function_instance(const function_declaration* declarer, const std::vector<std::shared_ptr<expression>>& expressions);

        [[nodiscard]] static bool is_instance_function() { return true; }
        [[nodiscard]] std::shared_ptr<element_object> call(const call_expression*) const override;
    };
}
