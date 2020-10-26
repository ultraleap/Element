#pragma once

//SELF
#include "object_model/object_internal.hpp"
#include "object_model/capture_stack.hpp"

namespace element
{
    class anonymous_block_instance final : public object, public std::enable_shared_from_this<anonymous_block_instance>
    {
    public:
        explicit anonymous_block_instance(const anonymous_block_expression* declarer, const std::map<identifier, object_const_shared_ptr>& named_expressions, source_information source_info);

        //[[nodiscard]] std::string typeof_info() const override;

        [[nodiscard]] bool is_constant() const override;
        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] virtual object_const_shared_ptr index(const compilation_context& context,
                                                            const identifier& name,
                                                            const source_information& source_info) const;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override;

        const anonymous_block_expression* const declarer;
        std::map<std::string, object_const_shared_ptr> fields;

    private:
        //mutable capture_stack captures;
    };
} // namespace element