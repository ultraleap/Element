#pragma once

//SELF
#include "scope.hpp"
#include "etree/fwd.hpp"

namespace element
{
    class struct_instance final : public object, public std::enable_shared_from_this<struct_instance>
    {
    public:
        explicit struct_instance(const struct_declaration* declarer);
        explicit struct_instance(const struct_declaration* declarer, const std::vector<object_const_shared_ptr>& expressions);
        virtual ~struct_instance() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        struct_instance(const struct_instance&) = delete;
        struct_instance(struct_instance&&) = delete;
        struct_instance& operator=(const struct_instance&) = delete;
        struct_instance& operator=(struct_instance&&) = delete;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context, const identifier& name,
                                                          const source_information& source_info) const override;
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const override;

        [[nodiscard]] std::shared_ptr<const element_expression> to_expression() const final;

        const struct_declaration* const declarer;
        std::map<std::string, object_const_shared_ptr> fields;

    private:
    };

    class function_instance final : public object, public std::enable_shared_from_this<function_instance>
    {
    public:
        explicit function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info);
        explicit function_instance(const function_declaration* declarer, capture_stack captures, source_information source_info, std::vector<object_const_shared_ptr> args);
        virtual ~function_instance() = default;

        //todo: default them if we really need them, but it's unlikely given it should be wrapped in a shared_ptr
        function_instance(const function_instance&) = delete;
        function_instance(function_instance&&) = delete;
        function_instance& operator=(const function_instance&) = delete;
        function_instance& operator=(function_instance&&) = delete;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                         std::vector<object_const_shared_ptr> compiled_args,
                                                         const source_information& source_info) const override;
        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                            const source_information& source_info) const override;

        const function_declaration* const declarer;

    private:
        mutable capture_stack captures;
        std::vector<object_const_shared_ptr> provided_arguments;
    };
}
