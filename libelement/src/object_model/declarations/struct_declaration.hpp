#pragma once

//SELF
#include "declaration.hpp"
#include "object_model/intermediaries/declaration_wrapper.hpp"
#include "object_model/constraints/user_type.hpp"

namespace element
{
    class struct_declaration final : public declaration
    {
    public:
        enum class kind
        {
            custom,
            intrinsic
        };

    public:
        struct_declaration(identifier name, const scope* parent_scope, kind struct_kind);

        [[nodiscard]] std::string typeof_info() const override;
        [[nodiscard]] std::string to_code(const int depth) const override;

        [[nodiscard]] bool matches_constraint(const compilation_context& context, const constraint* constraint) const override;
        [[nodiscard]] const constraint* get_constraint() const override;

        [[nodiscard]] object_const_shared_ptr index(const compilation_context& context, const identifier& name,
                                                    const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr call(const compilation_context& context,
                                                   std::vector<object_const_shared_ptr> compiled_args,
                                                   const source_information& source_info) const override;

        [[nodiscard]] object_const_shared_ptr compile(const compilation_context& context,
                                                      const source_information& source_info) const override { return wrapper; }

        [[nodiscard]] bool serializable(const compilation_context& context) const override;
        [[nodiscard]] bool deserializable(const compilation_context& context) const override;
        [[nodiscard]] object_const_shared_ptr generate_placeholder(const compilation_context& context, int& placeholder_index, unsigned int boundary_scope) const override;

        [[nodiscard]] bool is_intrinsic() const override;

    private:
        std::unique_ptr<user_type> type;
        kind struct_kind;
    };
} // namespace element