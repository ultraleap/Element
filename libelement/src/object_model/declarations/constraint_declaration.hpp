#pragma once

//SELF
#include "declaration.hpp"
#include "object_model/intermediaries/declaration_wrapper.hpp"

namespace element
{
    class constraint_declaration final : public declaration
    {
    public:
        constraint_declaration(identifier name, const scope* parent_scope, bool is_intrinsic);

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(int depth) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override { return wrapper; }

    private:
        std::unique_ptr<constraint> constraint_;
    };
}