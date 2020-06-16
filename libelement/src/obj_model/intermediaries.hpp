#pragma once

//SELF
#include "obj_model/scope.hpp"
#include "etree/fwd.hpp"

namespace element
{
    struct compiled_expression final : object
    {
        const object* creator = nullptr; //the thing that generated this thing
        //A compiled expression contains either an expression tree that was output from something being compiled, or some object model thing
        //probably shouldn't contain both but let's see if we ever need to :b
        std::shared_ptr<object> object_model;
        expression_shared_ptr expression_tree;
        //constraint_const_shared_ptr constraint = element_type::any; //the constraint of the expression/object model (if the object model isn't aware of its constraint? but it should be?)

        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&) const override;
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const override;
        [[nodiscard]] std::string to_string() const override { return ""; }
    };

    struct struct_instance final : object
    {
        const struct_declaration* const declarer;
        std::map<std::string, std::shared_ptr<compiled_expression>> fields;

        explicit struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<compiled_expression>>& expressions);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&) const override;
    };

    struct function_instance final : object
    {
        const function_declaration* const declarer;
        std::vector<std::shared_ptr<compiled_expression>> provided_arguments;

        explicit function_instance(const function_declaration* declarer, std::vector<std::shared_ptr<compiled_expression>> args);

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] static bool is_instance_function() { return true; }
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const override;
    };
}
