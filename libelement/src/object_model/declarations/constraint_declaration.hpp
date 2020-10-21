#pragma once

//SELF
#include "declaration.hpp"
#include "object_model/intermediaries/declaration_wrapper.hpp"

namespace element
{
    class constraint_declaration final : public declaration
    {
    public:
        enum class kind
        {
            custom,
            intrinsic
        };

    public:
        constraint_declaration(identifier name, const scope* parent_scope, kind constraint_kind);

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(const int depth) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override { return wrapper; }

        [[nodiscard]] bool is_intrinsic() const override;

    private:
        std::unique_ptr<constraint> constraint_;
        kind constraint_kind;
    };
} // namespace element