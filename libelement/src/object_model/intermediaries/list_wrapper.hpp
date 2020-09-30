#pragma once

//SELF
#include "object_model/object.hpp"
#include "etree/fwd.hpp"

namespace element
{
    //note: a list_wrapper means a runtime index, so all types are homogenous
    class list_wrapper final : public object, public std::enable_shared_from_this<list_wrapper>
    {
    public:
        static object_const_shared_ptr create_or_optimise(const object_const_shared_ptr& selector_object, const std::vector<object_const_shared_ptr>& option_objects, const source_information& source_info);

        explicit list_wrapper(std::shared_ptr<const element_expression> selector, std::vector<object_const_shared_ptr> options);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(const int depth = 0) const override;

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

        const std::shared_ptr<const element_expression> selector;
        const std::vector<object_const_shared_ptr> options;

    private:
    };
} // namespace element