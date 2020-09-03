#pragma once

//SELF
#include "object_model/object.hpp"
#include "etree/fwd.hpp"

namespace element
{
    //note: a for_wrapper means a runtime indexed for, so all types are homogenous
    class for_wrapper final : public object, public std::enable_shared_from_this<for_wrapper> {
    public:
        static object_const_shared_ptr create_or_optimise(const object_const_shared_ptr& initial_object,
                                                          const object_const_shared_ptr& predicate_object,
                                                          const object_const_shared_ptr& body_object,
                                                          const source_information& source_info);

        explicit for_wrapper(const std::shared_ptr<const element_expression>& initial,
                             const std::shared_ptr<const element_expression>& predicate,
                             const std::shared_ptr<const element_expression>& body);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth = 0) const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
            std::vector<object_const_shared_ptr> compiled_args,
            const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context,
            const identifier& name,
            const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
            const source_information& source_info) const override;

        [[nodiscard]] std::shared_ptr<const element_expression> to_expression() const final;

    private:
        std::shared_ptr<object_const_shared_ptr> initial;
        std::shared_ptr<object_const_shared_ptr> predicate;
        std::shared_ptr<object_const_shared_ptr> body;
    };
}