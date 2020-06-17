#pragma once

//SELF
#include "scope.hpp"
#include "etree/fwd.hpp"

namespace element
{
    class compiled_expression final : public object, public std::enable_shared_from_this<compiled_expression>
    {
    public:
        compiled_expression() = default;
        virtual ~compiled_expression() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        compiled_expression(const compiled_expression&) = delete;
        compiled_expression(compiled_expression&&) = delete;
        compiled_expression& operator=(const compiled_expression&) = delete;
        compiled_expression& operator=(compiled_expression&&) = delete;

        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&) const override;
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const override;
        [[nodiscard]] virtual std::shared_ptr<compiled_expression> compile(const compilation_context& context) const;
        [[nodiscard]] std::string to_string() const override { return ""; }

        const object* creator = nullptr; //the thing that generated this thing
        type_const_shared_ptr type = nullptr; //the type of the thing that generated this thing
        //A compiled expression contains either an expression tree that was output from something being compiled, or some object model thing
        //probably shouldn't contain both but let's see if we ever need to :b
        std::shared_ptr<object> object_model;
        expression_shared_ptr expression_tree;
        //constraint_const_shared_ptr constraint = element_type::any; //the constraint of the expression/object model (if the object model isn't aware of its constraint? but it should be?)
    private:
    };

    class struct_instance final : public object
    {
    public:
        explicit struct_instance(const struct_declaration* declarer, const std::vector<std::shared_ptr<compiled_expression>>& expressions);
        virtual ~struct_instance() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        struct_instance(const struct_instance&) = delete;
        struct_instance(struct_instance&&) = delete;
        struct_instance& operator=(const struct_instance&) = delete;
        struct_instance& operator=(struct_instance&&) = delete;

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] std::shared_ptr<object> index(const compilation_context& context, const identifier&) const override;

        const struct_declaration* const declarer;
        std::map<std::string, std::shared_ptr<compiled_expression>> fields;

    private:
    };

    class function_instance final : public object
    {
    public:
        explicit function_instance(const function_declaration* declarer, std::vector<std::shared_ptr<compiled_expression>> args);
        virtual ~function_instance() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        function_instance(const function_instance&) = delete;
        function_instance(function_instance&&) = delete;
        function_instance& operator=(const function_instance&) = delete;
        function_instance& operator=(function_instance&&) = delete;

        [[nodiscard]] std::string to_string() const override;
        [[nodiscard]] static bool is_instance_function() { return true; }
        [[nodiscard]] std::shared_ptr<object> call(const compilation_context& context, std::vector<std::shared_ptr<compiled_expression>> args) const override;

        const function_declaration* const declarer;
        std::vector<std::shared_ptr<compiled_expression>> provided_arguments;

    private:
    };
}
